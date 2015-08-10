/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Transaction.cc
 */
extern "C"
{
#include <solv/transaction.h>
#include <solv/solver.h>
}
#include <iostream>
#include "zypp/base/LogTools.h"
#include "zypp/base/SerialNumber.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Hash.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Transaction.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/Queue.h"
#include "zypp/sat/Map.h"
#include "zypp/ResPool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    /** Transaction implementation.
     *
     * \NOTE After commit the @System repo is reloaded. This invalidates
     * the ids off all installed items in the transaction, including their
     * stepType. Thats why some information (stepType, NVRA) is be stored
     * for post mortem access (i.e. tell after commit which NVRA were deleted).
     *
     */
    struct Transaction::Impl : protected detail::PoolMember
			     , private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const Impl & obj );

      public:
	typedef std::unordered_set<detail::IdType> set_type;
	typedef std::unordered_map<detail::IdType,detail::IdType> map_type;

	struct PostMortem
	{
	  PostMortem()
	  {}
	  PostMortem( const sat::Solvable & solv_r )
	    : _ident( solv_r.ident() )
	    , _edition( solv_r.edition() )
	    , _arch( solv_r.arch() )
	  {}

	  IdString _ident;
	  Edition  _edition;
	  Arch     _arch;
	};
	typedef std::unordered_map<detail::IdType,PostMortem> pmmap_type;

      public:
	Impl()
	  : _trans( ::transaction_create( nullptr ) )
	{ memset( _trans, 0, sizeof(*_trans) ); }

	Impl( LoadFromPoolType )
	  : _watcher( myPool().serial() )
	  , _trans( nullptr )
	{
	  Queue decisionq;
	  for ( const PoolItem & pi : ResPool::instance() )
	  {
	    if ( ! pi.status().transacts() )
	      continue;
	    decisionq.push( pi.isSystem() ? -pi.id() : pi.id() );
	  }
	  Queue noobsq;
	  for ( const Solvable & solv : myPool().multiversionList() )
	  {
	    noobsq.push( SOLVER_NOOBSOLETES | SOLVER_SOLVABLE );
	    noobsq.push( solv.id() );
	  }
	  Map noobsmap;
	  ::solver_calculate_noobsmap( myPool().getPool(), noobsq, noobsmap );
	  _trans = ::transaction_create_decisionq( myPool().getPool(), decisionq, noobsmap );

	  // NOTE: package/product buddies share the same ResStatus
	  // so we also link the buddies stepStages. This assumes
	  // only one buddy is acting during commit (package is installed,
	  // but no extra operation for the product).
	  for_( it, _trans->steps.elements, _trans->steps.elements + _trans->steps.count )
	  {
	    sat::Solvable solv( *it );
	    // buddy list:
	    if ( ! solv.isKind<Package>() )
	    {
	      PoolItem pi( solv );
	      if ( pi.buddy() )
	      {
		_linkMap[*it] = pi.buddy().id();
	      }
	    }
	    if ( solv.isSystem() )
	    {
	      // to delete list:
	      if ( stepType( solv ) == TRANSACTION_ERASE )
	      {
		_systemErase.insert( *it );
	      }
	      // post mortem data
	      _pmMap[*it] = solv;
	    }
	  }
	}

	~Impl()
	{ ::transaction_free( _trans ); }

      public:
	bool valid() const
	{ return _watcher.isClean( myPool().serial() ); }

	bool order()
	{
	  if ( ! valid() )
	    return false;
	  if ( empty() )
	    return true;
#if 0
	  // This is hwo we could implement out own order method.
	  // As ::transaction already groups by MediaNr, we don't
	  // need it for ORDER_BY_MEDIANR.
	  ::transaction_order( _trans, SOLVER_TRANSACTION_KEEP_ORDERDATA );
	  detail::IdType chosen = 0;
	  Queue choices;

	  while ( true )
	  {
	    int ret = transaction_order_add_choices( _trans, chosen, choices );
	    MIL << ret << ": " << chosen << ": " << choices << endl;
	    chosen = choices.pop_front(); // pick one out of choices
	    if ( ! chosen )
	      break;
	  }
	  return true;
#endif
	  if ( !_ordered )
	  {
	    ::transaction_order( _trans, 0 );
	    _ordered = true;
	  }
	  return true;
	}

	bool empty() const
	{ return( _trans->steps.count == 0 ); }

	size_t size() const
	{ return _trans->steps.count; }

	const_iterator begin( const RW_pointer<Transaction::Impl> & self_r ) const
	{ return const_iterator( self_r, _trans->steps.elements ); }
	iterator begin( const RW_pointer<Transaction::Impl> & self_r )
	{ return iterator( self_r, _trans->steps.elements ); }

	const_iterator end( const RW_pointer<Transaction::Impl> & self_r ) const
	{ return const_iterator( self_r, _trans->steps.elements + _trans->steps.count ); }
	iterator end( const RW_pointer<Transaction::Impl> & self_r )
	{ return iterator( self_r, _trans->steps.elements + _trans->steps.count ); }

	const_iterator find(const RW_pointer<Transaction::Impl> & self_r, const sat::Solvable & solv_r ) const
	{ detail::IdType * it( _find( solv_r ) ); return it ? const_iterator( self_r, it ) : end( self_r ); }
	iterator find(const RW_pointer<Transaction::Impl> & self_r, const sat::Solvable & solv_r )
	{ detail::IdType * it( _find( solv_r ) ); return it ? iterator( self_r, it ) : end( self_r ); }

      public:
	int installedResult( Queue & result_r ) const
	{ return ::transaction_installedresult( _trans, result_r ); }

	StringQueue autoInstalled() const
	{ return _autoInstalled; }

	void autoInstalled( const StringQueue & queue_r )
	{ _autoInstalled = queue_r; }

      public:
	StepType stepType( Solvable solv_r ) const
	{
	  if ( ! solv_r )
	  {
	    // post mortem @System solvable
	    return isIn( _systemErase, solv_r.id() ) ? TRANSACTION_ERASE : TRANSACTION_IGNORE;
	  }

	  switch( ::transaction_type( _trans, solv_r.id(), SOLVER_TRANSACTION_RPM_ONLY ) )
	  {
	    case SOLVER_TRANSACTION_ERASE: return TRANSACTION_ERASE; break;
	    case SOLVER_TRANSACTION_INSTALL: return TRANSACTION_INSTALL; break;
	    case SOLVER_TRANSACTION_MULTIINSTALL: return TRANSACTION_MULTIINSTALL; break;
	  }
	  return TRANSACTION_IGNORE;
	}

	StepStage stepStage( Solvable solv_r ) const
	{ return stepStage( resolve( solv_r ) ); }

	void stepStage( Solvable solv_r, StepStage newval_r )
	{ stepStage( resolve( solv_r ), newval_r ); }

	const PostMortem & pmdata( Solvable solv_r ) const
	{
	  static PostMortem _none;
	  pmmap_type::const_iterator it( _pmMap.find( solv_r.id() ) );
	  return( it == _pmMap.end() ? _none : it->second );
	}

      private:
	detail::IdType resolve( const Solvable & solv_r ) const
	{
	  map_type::const_iterator res( _linkMap.find( solv_r.id() ) );
	  return( res == _linkMap.end() ? solv_r.id() : res->second );
	}

	bool isIn( const set_type & set_r, detail::IdType sid_r ) const
	{ return( set_r.find( sid_r ) != set_r.end() ); }

	StepStage stepStage( detail::IdType sid_r ) const
	{
	  if ( isIn( _doneSet, sid_r ) )
	    return STEP_DONE;
	  if ( isIn( _errSet, sid_r ) )
	    return STEP_ERROR;
	  return STEP_TODO;
	}

	void stepStage( detail::IdType sid_r, StepStage newval_r )
	{
	  StepStage stage( stepStage( sid_r ) );
	  if ( stage != newval_r )
	  {
	    // reset old stage
	    if ( stage != STEP_TODO )
	    {
	      (stage == STEP_DONE ? _doneSet : _errSet).erase( sid_r );
	    }
	    if ( newval_r != STEP_TODO )
	    {
	      (newval_r == STEP_DONE ? _doneSet : _errSet).insert( sid_r );
	    }
	  }
	}

      private:
	detail::IdType * _find( const sat::Solvable & solv_r ) const
	{
	  if ( solv_r && _trans->steps.elements )
	  {
	    for_( it, _trans->steps.elements, _trans->steps.elements + _trans->steps.count )
	    {
	      if ( *it == detail::IdType(solv_r.id()) )
		return it;
	    }
	  }
	  return 0;
	}

     private:
	SerialNumberWatcher _watcher;
	mutable ::Transaction * _trans;
	DefaultIntegral<bool,false> _ordered;
	//
	set_type	_doneSet;
	set_type	_errSet;
	map_type	_linkMap;	// buddy map to adopt buddies StepResult
	set_type	_systemErase;	// @System packages to be eased (otherse are TRANSACTION_IGNORE)
	pmmap_type	_pmMap;		// Post mortem data of deleted @System solvables

	StringQueue	_autoInstalled;	// ident strings of all packages that would be auto-installed after the transaction is run.

      public:
        /** Offer default Impl. */
        static shared_ptr<Impl> nullimpl()
        {
          static shared_ptr<Impl> _nullimpl( new Impl );
          return _nullimpl;
        }
    };

    /** \relates Transaction::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Transaction::Impl & obj )
    {
      return str << "Transaction: " << obj.size() << " (" << (obj.valid()?"valid":"INVALID") << ")";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Transaction
    //
    ///////////////////////////////////////////////////////////////////

    Transaction::Transaction()
      : _pimpl( Impl::nullimpl() )
    {}

    Transaction::Transaction( LoadFromPoolType )
      : _pimpl( new Impl( loadFromPool ) )
    {}

    Transaction::~Transaction()
    {}

    bool Transaction::valid() const
    { return _pimpl->valid(); }

    bool Transaction::order()
    { return _pimpl->order(); }

    bool Transaction::empty() const
    { return _pimpl->empty(); }

    size_t Transaction::size() const
    { return _pimpl->size(); }

    Transaction::const_iterator Transaction::begin() const
    { return _pimpl->begin( _pimpl ); }

    Transaction::iterator Transaction::begin()
    { return _pimpl->begin( _pimpl ); }

    Transaction::const_iterator Transaction::end() const
    { return _pimpl->end( _pimpl ); }

    Transaction::iterator Transaction::end()
    { return _pimpl->end( _pimpl ); }

    Transaction::const_iterator Transaction::find( const sat::Solvable & solv_r ) const
    { return _pimpl->find( _pimpl, solv_r ); }

    Transaction::iterator Transaction::find( const sat::Solvable & solv_r )
    { return _pimpl->find( _pimpl, solv_r ); }

    int Transaction::installedResult( Queue & result_r ) const
    { return _pimpl->installedResult( result_r ); }

    StringQueue Transaction::autoInstalled() const
    { return _pimpl->autoInstalled(); }

    void Transaction::autoInstalled( const StringQueue & queue_r )
    { _pimpl->autoInstalled( queue_r ); }

    std::ostream & operator<<( std::ostream & str, const Transaction & obj )
    { return str << *obj._pimpl; }

    std::ostream & dumpOn( std::ostream & str, const Transaction & obj )
    {
      for_( it, obj.begin(), obj.end() )
      {
	str << *it << endl;
      }
      return str;
    }

    bool operator==( const Transaction & lhs, const Transaction & rhs )
    { return lhs._pimpl == rhs._pimpl; }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Transaction::Step
    //
    ///////////////////////////////////////////////////////////////////

    Transaction::Step::Step()
    {}

    Transaction::StepType Transaction::Step::stepType() const
    { return _pimpl->stepType( _solv ); }

    Transaction::StepStage Transaction::Step::stepStage() const
    { return _pimpl->stepStage( _solv ); }

    void Transaction::Step::stepStage( StepStage val_r )
    { _pimpl->stepStage( _solv, val_r ); }

    IdString Transaction::Step::ident() const
    { return _solv ? _solv.ident() : _pimpl->pmdata(_solv )._ident; }

    Edition Transaction::Step::edition() const
    { return _solv ? _solv.edition() : _pimpl->pmdata(_solv )._edition; }

    Arch Transaction::Step::arch() const
    { return _solv ? _solv.arch() : _pimpl->pmdata(_solv )._arch; }

    std::ostream & operator<<( std::ostream & str, const Transaction::Step & obj )
    {
      str << obj.stepType() << obj.stepStage() << " ";
      if ( obj.satSolvable() )
	str << PoolItem( obj.satSolvable() );
      else
	str << '[' << obj.ident() << '-' << obj.edition() << '.' << obj.arch() << ']';
      return str;
    }

    std::ostream & operator<<( std::ostream & str, Transaction::StepType obj )
    {
      switch ( obj )
      {
	#define OUTS(E,S) case Transaction::E: return str << #S; break
	OUTS( TRANSACTION_IGNORE,	[ ] );
	OUTS( TRANSACTION_ERASE,	[-] );
	OUTS( TRANSACTION_INSTALL,	[+] );
	OUTS( TRANSACTION_MULTIINSTALL,	[M] );
	#undef OUTS
      }
      return str << "[?]";
    }

    std::ostream & operator<<( std::ostream & str, Transaction::StepStage obj )
    {
      switch ( obj )
      {
	#define OUTS(E,S) case Transaction::E: return str << #S; break
	OUTS( STEP_TODO,	[__] );
	OUTS( STEP_DONE,	[OK] );
	OUTS( STEP_ERROR,	[**] );
	#undef OUTS
      }
      return str << "[??]";
    }
    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Transaction::const_iterator/iterator
      //
      ///////////////////////////////////////////////////////////////////

      Transaction_const_iterator::Transaction_const_iterator()
      : Transaction_const_iterator::iterator_adaptor_( 0 )
      {}

      Transaction_const_iterator::Transaction_const_iterator( const Transaction_iterator & iter_r )
      : Transaction_const_iterator::iterator_adaptor_( iter_r.base() )
      , _pimpl( iter_r._pimpl )
      {}

      Transaction_iterator::Transaction_iterator()
      : Transaction_iterator::iterator_adaptor_( 0 )
      {}

      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
