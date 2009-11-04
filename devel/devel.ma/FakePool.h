#if 0
#define FakePool_h

#include <iostream>
#include <vector>
#include <string>

#include "zypp/base/LogTools.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/Function.h"
#include "zypp/base/Functional.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/String.h"

#include "zypp/ZYppFactory.h"
#include "zypp/ResPool.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/CapFactory.h"

#include "zypp/Atom.h"
#include "zypp/Package.h"
#include "zypp/SrcPackage.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"
#include "zypp/Product.h"
#include "zypp/Patch.h"
#include "zypp/Script.h"
#include "zypp/Message.h"
#include "zypp/Language.h"
#include "zypp/VendorAttr.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace debug
  { /////////////////////////////////////////////////////////////////

    /**
     * \code
     * const char * data[] = {
     * "@ product"
     * ,"@ installed"
     * ,"- prodold 1 1 x86_64"
     * ,"@ available"
     * ,"- prodnew 1 1 x86_64"
     * ,"@ obsoletes"
     * ,"prodold"
     * ,"@ fin"
     * };
     * DataCollect dataCollect;
     * for_each( data, data + ( sizeof(data) / sizeof(const char *) ),
     * function<void(const string &)>( ref( dataCollect ) ) );
     * \endcode
    */
    class DataCollect
    {
    public:
      DataCollect( bool verbose_r = true )
      : _definst( false )
      , _defkind( ResKind::package )
      , _defdep( Dep::PROVIDES )
      , _defdepref( _defkind )
      , _verbose( verbose_r )
      {
	VendorAttr::disableAutoProtect();
      }

      bool operator()( const std::string & line_r )
      {
	parseLine( str::trim( line_r ) );
	return true;
      }

      const ResStore & installed() const
      { return _installed; }

      const ResStore & available() const
      { return _available; }

      template<class _Iterator>
        void collect( _Iterator begin_r, _Iterator end_r )
        {
          for_each( begin_r, end_r,
                    function<void(const std::string &)>( ref(*this) ) );
        }

    private:
      struct Data
      {
        Data( bool inst_r, Resolvable::Kind kind_r, const std::vector<std::string> & words_r )
        : _inst( inst_r )
        , _kind( kind_r )
        , _data( words_r[1], Edition( words_r[2], words_r[3] ), Arch( words_r[4] ) )
        {}

        bool             _inst;
        Resolvable::Kind _kind;
        NVRAD            _data;
      };

    private:
      void parseLine( const std::string & line_r )
      {
	if ( line_r.empty() || line_r[0] == '#' )
	  return;

	std::vector<std::string> words;
        str::split( line_r, std::back_inserter( words ) );
        if ( words.empty() )
          return;

        if ( words[0] == "@" )
          {
            if ( words.size() < 2 )
              throw line_r;
            if ( words[1] == "installed" )
              _definst = true;
            else if ( words[1] == "available" )
              _definst = false;
            else if ( words[1] == "fin" )
              finalize();
            else
              {
                try
                  {
                    _defdep = Dep( words[1] );
                    if ( words.size() > 2 )
                      _defdepref = Resolvable::Kind( words[2] );
                  }
                catch ( ... )
                  {
                    _defkind = _defdepref = Resolvable::Kind( words[1] );
                  }
                return;
              }
          }
        else if ( words[0] == "-" )
          {
            if ( words.size() == 5 )
              {
                finalize();
                _d.reset( new Data( _definst, _defkind, words ) );
              }
            else
              {
                throw words;
              }
          }
        else
          {
            _d->_data[_defdep].insert( CapFactory().parse( _defdepref, line_r ) );
          }
      }

      void finalize()
      {
        if ( _d )
          {
            ResObject::Ptr p;
            if ( _d->_kind == ResKind::package )
              p = make<Package>();
            else if ( _d->_kind == ResKind::srcpackage )
              p = make<SrcPackage>();
            else if ( _d->_kind == ResTraits<Selection>::kind )
              p = make<Selection>();
            else if ( _d->_kind == ResKind::pattern )
              p = make<Pattern>();
            else if ( _d->_kind == ResKind::product )
              p = make<Product>();
            else if ( _d->_kind == ResKind::patch )
              p = make<Patch>();
            else if ( _d->_kind == ResTraits<Script>::kind )
              p = make<Script>();
            else if ( _d->_kind == ResTraits<Message>::kind )
              p = make<Message>();
            else if ( _d->_kind == ResTraits<Language>::kind )
              p = make<Language>();
            else if ( _d->_kind == ResTraits<Atom>::kind )
              p = make<Atom>();
            else
              throw _d->_kind;

            if ( _verbose )
              {
                _MIL("FakePool") << (_d->_inst?"i":"a") << " " << p << std::endl;
                _DBG("FakePool") << p->deps() << std::endl;
              }

            (_d->_inst?_installed:_available).insert( p );
            _d.reset();
          }
      }

      template<class _Res>
        ResObject::Ptr make()
        {
          typename detail::ResImplTraits<typename _Res::Impl>::Ptr impl;
          return zypp::detail::makeResolvableAndImpl( _d->_data, impl );
        }

    private:
      bool             _definst;
      Resolvable::Kind _defkind;
      Dep              _defdep;
      Resolvable::Kind _defdepref;

      bool             _verbose;

      shared_ptr<Data> _d;

      ResStore         _installed;
      ResStore         _available;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates DataCollect Stream output. */
    inline std::ostream & operator<<( std::ostream & str, const DataCollect & obj )
    {
      dumpRange( str << "Installed" << endl,
                 obj.installed().begin(),
                 obj.installed().end() ) << endl;
      dumpRange( str << "Available:" << endl,
                 obj.available().begin(),
                 obj.available().end() ) << endl;
      return str;
    }

    ///////////////////////////////////////////////////////////////////

    template<class _Iterator>
	inline void addPool( _Iterator begin_r, _Iterator end_r )
    {
      DataCollect dataCollect;
      dataCollect.collect( begin_r, end_r );
      getZYpp()->addResolvables( dataCollect.installed(), true );
      getZYpp()->addResolvables( dataCollect.available() );
    }

    inline void addPool( const Pathname & file_r )
    {
      std::ifstream in( file_r.c_str() );
      DataCollect dataCollect;
      function<bool(const std::string &)> fnc( ref(dataCollect) );
      iostr::forEachLine( in, fnc );
      getZYpp()->addResolvables( dataCollect.installed(), true );
      getZYpp()->addResolvables( dataCollect.available() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace debug
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // FakePool_h
