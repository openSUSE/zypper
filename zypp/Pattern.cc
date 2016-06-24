/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Pattern.cc
 *
*/
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/ResPool.h"
#include "zypp/Pattern.h"
#include "zypp/Filter.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace
  {
    inline Capability autoCapability( const Capabilities & provides_r )
    {
      static const Capability autopattern( "autopattern()" );
      for ( const auto & cap : provides_r )
	if ( cap.matches( autopattern ) == CapMatch::yes )
	  return cap;
      return Capability();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatternExpander
    //
    /** Recursively expand a Pattern.
     *
     * This means recursively expanding Patterns included by this or
     * extending this. The result is a \c set of <tt>Pattern::constPtr</tt>
     * accessible via iterator.
     */
    class PatternExpander
    {
      public:
        typedef std::map<Pattern::constPtr, DefaultIntegral<bool, false> > PatternMap;
        typedef PatternMap::size_type size_type;
        typedef PatternMap::key_type  value_type;
        typedef MapKVIteratorTraits<PatternMap>::Key_const_iterator const_iterator;

      public:
        PatternExpander()
        {}

        /** Recursively expand Pattern. */
        size_type doExpand( Pattern::constPtr pat_r )
        {
          // INT << "+++ " << pat_r << " ++++++++++++++++++++++++++++++++++" << endl;
          _patternMap.clear();
          if ( pat_r )
          {
            _patternMap[pat_r];
            Pattern::constPtr unprocessed( pat_r );
            // MIL << _patternMap << endl;
            do {
              expandIncludes( unprocessed );
              expandExtending( unprocessed );
              _patternMap[unprocessed] = true;
              // MIL << _patternMap << endl;
            } while( (unprocessed = nextUnprocessed()) );
          }
          // SEC << "--- " << _patternMap.size() << " ----------------------------------" << endl;
          return _patternMap.size();
        }

        const_iterator begin() const
        { return make_map_key_begin( _patternMap ); }

        const_iterator end() const
        { return make_map_key_end( _patternMap ); }

      private:
        /** Get the next unprocessed Pattern in \c _patternMap. */
        Pattern::constPtr nextUnprocessed() const
        {
          for_( it, _patternMap.begin(), _patternMap.end() )
          {
            if ( ! it->second )
              return it->first;
          }
          return NULL;
        }

      private:
        /** Store all included patterns in \c _patternMap. */
        void expandIncludes( const Pattern::constPtr & pat_r )
        {
          Pattern::NameList c( pat_r->includes() );
          for_( it, c.begin(), c.end() )
          {
            expandInclude( Capability( it->c_str()/*, *ResKind::pattern*/ ) );
          }
        }

        /** Store Patterns matching an \c Includes capability in \c _patternMap. */
        void expandInclude( const Capability & include_r )
        {
          sat::WhatProvides w( include_r );
          for_( it, w.begin(), w.end() )
          {
            _patternMap[asKind<Pattern>(PoolItem(*it))];
          }
        }

      private:
        /** Store all patterns extending \c pat_r in \c _patternMap. */
        void expandExtending( const Pattern::constPtr & pat_r )
        {
          ResPool pool( ResPool::instance() );
          for_( it, pool.byKindBegin<Pattern>(), pool.byKindEnd<Pattern>() )
          {
            expandIfExtends( pat_r, *it );
          }
        }

        /** Store \c extending_r if it extends \c pat_r. */
        void expandIfExtends( const Pattern::constPtr & pat_r, const PoolItem & extending_r )
        {
          Pattern::constPtr extending( asKind<Pattern>(extending_r) );
          Pattern::NameList c( extending->extends() );
          for_( it, c.begin(), c.end() )
          {
            if ( providedBy( pat_r, Capability( it->c_str()/*, *ResKind::pattern*/ ) ) )
            {
              // an extends matches the Pattern
              _patternMap[extending];
              break;
            }
          }
        }

        /** Return true if Capability \c extends_r is provided by Pattern. */
        bool providedBy( const Pattern::constPtr & pat_r, const Capability & extends_r )
        {
          if ( !pat_r )
            return false;

          sat::Solvable pat( pat_r->satSolvable() );
          sat::WhatProvides w( extends_r );
          for_( it, w.begin(), w.end() )
          {
            if ( pat == *it )
              return true;
          }
          return false;
        }

      private:
        PatternMap _patternMap;
    };
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //	Pattern
  ///////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Pattern);

  Pattern::Pattern( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
  {}

  Pattern::~Pattern()
  {}

  bool Pattern::isDefault() const
  { return lookupBoolAttribute( sat::SolvAttr::isdefault ); }

  bool Pattern::userVisible() const
  {
    // bsc#900769: If visibility is a string(solvable ident) the pattern
    // is visible IFF ident is available in the pool.
    IdString ident( lookupStrAttribute( sat::SolvAttr::isvisible ) );
    return( ident.empty() ? lookupBoolAttribute( sat::SolvAttr::isvisible )
			  : ! ResPool::instance().byIdent( ident ).empty() );
  }

  std::string Pattern::category( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::category, lang_r ); }

  Pathname Pattern::icon() const
  { return lookupStrAttribute( sat::SolvAttr::icon ); }

  Pathname Pattern::script() const
  { return lookupStrAttribute( sat::SolvAttr::script ); }

  std::string Pattern::order() const
  { return lookupStrAttribute( sat::SolvAttr::order ); }

  bool Pattern::isAutoPattern() const
  { return bool(autoCapability( provides() )); }

  sat::Solvable Pattern::autoPackage() const
  {
    Capability autocap( autoCapability( provides() ) );
    if ( autocap )
    {
      Capability pkgCap( arch(), autocap.detail().ed().asString(), Rel::EQ, edition() );
      for ( const auto & solv: sat::WhatProvides( pkgCap ) )
	if ( solv.repository() == repository() )
	  return solv;
    }
    return sat::Solvable();
  }

  Pattern::NameList Pattern::includes() const
  { return NameList( sat::SolvAttr::includes, satSolvable() ); }

  Pattern::NameList Pattern::extends() const
  { return NameList( sat::SolvAttr::extends, satSolvable() ); }

  ///////////////////////////////////////////////////////////////////
  namespace
  {
    inline void addCaps( CapabilitySet & caps_r, sat::Solvable solv_r, Dep dep_r )
    {
      Capabilities c( solv_r[dep_r] );
      if ( ! c.empty() )
      {
	caps_r.insert( c.begin(),c.end() );
      }
    }
  } //namespace
  ///////////////////////////////////////////////////////////////////

  Pattern::Contents Pattern::core() const
  {
    // Content dependencies are either associated with
    // the autoPackage or the (oldstype) pattern itself.
    // load requires
    CapabilitySet caps;
    addCaps( caps, *this, Dep::REQUIRES );

    sat::Solvable depKeeper( autoPackage() );
    if ( depKeeper )
      addCaps( caps, depKeeper, Dep::REQUIRES );
    // get items providing the requirements
    sat::WhatProvides prv( caps );
    // return packages only.
    return Pattern::Contents( make_filter_begin( filter::byKind<Package>(), prv ),
                              make_filter_end( filter::byKind<Package>(), prv ) );
  }

  Pattern::Contents Pattern::depends( bool includeSuggests_r ) const
  {
    // Content dependencies are either associated with
    // the autoPackage or the (oldstype) pattern itself.
    // load requires, recommends[, suggests]
    CapabilitySet caps;
    addCaps( caps, *this, Dep::REQUIRES );
    addCaps( caps, *this, Dep::RECOMMENDS );
    if ( includeSuggests_r )
      addCaps( caps, *this, Dep::SUGGESTS );

    sat::Solvable depKeeper( autoPackage() );
    if ( depKeeper )
    {
      addCaps( caps, depKeeper, Dep::REQUIRES );
      addCaps( caps, depKeeper, Dep::RECOMMENDS );
      if ( includeSuggests_r )
	addCaps( caps, depKeeper, Dep::SUGGESTS );
    }
    // get items providing the above
    sat::WhatProvides prv( caps );
    // return packages only.
    return Pattern::Contents( make_filter_begin( filter::byKind<Package>(), prv ),
                              make_filter_end( filter::byKind<Package>(), prv ) );
  }

  Pattern::Contents Pattern::contents( bool includeSuggests_r ) const
  {
    PatternExpander expander;
    if ( ! expander.doExpand( this ) )
      return Contents(); // empty pattern set

    Contents result;
    for_( it, expander.begin(), expander.end() )
    {
      Contents c( (*it)->depends( includeSuggests_r ) );
      result.get().insert( c.begin(), c.end() );
    }
    return result;
  }

  ///////////////////////////////////////////////////////////////////
  namespace
  {
    // Get packages referenced by depKeeper dependency.
    inline void dependsSetDoCollect( sat::Solvable depKeeper_r, Dep dep_r, Pattern::Contents & set_r )
    {
      CapabilitySet caps;
      addCaps( caps, depKeeper_r, dep_r );
      sat::WhatProvides prv( caps );
      for ( ui::Selectable::Ptr sel : prv.selectable() )
      {
	const PoolItem & pi( sel->theObj() );
	if ( pi.isKind<Package>() )
	  set_r.insert( pi );
      }
    }

    // Get packages referenced by depKeeper.
    inline void dependsSet( sat::Solvable depKeeper_r, Pattern::ContentsSet & collect_r )
    {
      dependsSetDoCollect( depKeeper_r, Dep::REQUIRES,	 collect_r.req );
      dependsSetDoCollect( depKeeper_r, Dep::RECOMMENDS, collect_r.rec ),
      dependsSetDoCollect( depKeeper_r, Dep::SUGGESTS,	 collect_r.sug );
    }

    // Whether this is a patterns depkeeper.
    inline bool isPatternsPackage( sat::Solvable depKeeper_r )
    {
      static const Capability indicator( "pattern()" );
      return depKeeper_r.provides().matches( indicator );
    }
  } // namespace
  ///////////////////////////////////////////////////////////////////
  void Pattern::contentsSet( ContentsSet & collect_r, bool recursively_r ) const
  {
    sat::Solvable depKeeper( autoPackage() );	// (my required) patterns-package
    if ( ! depKeeper )
      return;

    // step 2 data
    std::set<sat::Solvable> recTodo;	// recommended patterns-packages to process
    std::set<sat::Solvable> allDone;	// patterns-packages already expanded
    {
      // step 1: Expand requirements, remember recommends....
      // step 1 data (scoped to step1)
      std::set<sat::Solvable> reqTodo;	// required patterns-packages to process

      collect_r.req.insert( depKeeper );// collect the depKeeper
      reqTodo.insert( depKeeper );	// and expand it...

      while ( ! reqTodo.empty() )
      {
	// pop one patterns-package from todo
	depKeeper = ( *reqTodo.begin() );
	reqTodo.erase( reqTodo.begin() );
	allDone.insert( depKeeper );

	// collects stats
	ContentsSet result;
	dependsSet( depKeeper, result );

	// evaluate result....
	for ( sat::Solvable solv : result.req )	// remember unprocessed required patterns-packages...
	{
	  if ( collect_r.req.insert( solv ) && recursively_r && isPatternsPackage( solv ) )
	    reqTodo.insert( solv );
	}
	for ( sat::Solvable solv : result.rec )	// remember unprocessed recommended patterns-packages...
	{
	  if ( collect_r.rec.insert( solv ) && recursively_r && isPatternsPackage( solv ) )
	    recTodo.insert( solv );
	}
	for ( sat::Solvable solv : result.sug )	// NOTE: We don't expand suggested patterns!
	{
	  collect_r.sug.insert( solv );
	}
      }
    }
    // step 2: All requirements are expanded, now check remaining recommends....
    while ( ! recTodo.empty() )
    {
      // pop one patterns-package from todo
      depKeeper = ( *recTodo.begin() );
      recTodo.erase( recTodo.begin() );
      if ( ! allDone.insert( depKeeper ).second )
	continue;	// allready expanded (in requires)

      // collects stats
      ContentsSet result;
      dependsSet( depKeeper, result );

      // evaluate result....
      for ( sat::Solvable solv : result.req )	// remember unprocessed required patterns-packages...
      {
	// NOTE: Requirements of recommended patterns count as 'recommended'
	if ( collect_r.rec.insert( solv ) && recursively_r && isPatternsPackage( solv ) )
	  recTodo.insert( solv );
      }
      for ( sat::Solvable solv : result.rec )	// remember unprocessed recommended patterns-packages...
      {
	if ( collect_r.rec.insert( solv ) && recursively_r && isPatternsPackage( solv ) )
	  recTodo.insert( solv );
      }
	for ( sat::Solvable solv : result.sug )	// NOTE: We don't expand suggested patterns!
	{
	  collect_r.sug.insert( solv );
	}
    }
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
