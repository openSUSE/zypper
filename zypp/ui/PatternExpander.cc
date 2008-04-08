/*---------------------------------------------------------------------\
 |                          ____ _   __ __ ___                          |
 |                         |__  / \ / / . \ . \                         |
 |                           / / \ V /|  _/  _/                         |
 |                          / /__ | | | | | |                           |
 |                         /_____||_| |_| |_|                           |
 |                                                                      |
 \---------------------------------------------------------------------*/
/** \file	zypp/ui/PatternExpander.cc
 *
*/
#include <iostream>
//#include "zypp/base/LogTools.h"

#include "zypp/ui/PatternExpander.h"

#include "zypp/base/Algorithm.h"
#include "zypp/base/Function.h"
#include "zypp/ResPool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatternExpander::Impl
    //
    /** PatternExpander implementation. */
    class PatternExpander::Impl
    {
    public:
      Impl( const ResPool & pool_r )
      : _pool( pool_r )
      {}

      /** Recursively expand Pattern. */
      size_type doExpand( Pattern::constPtr pat_r )
      {
        //INT << "+++ " << pat_r << " ++++++++++++++++++++++++++++++++++" << endl;
        _patternMap.clear();
        if ( pat_r )
          {
            _patternMap[pat_r];
            Pattern::constPtr unprocessed( pat_r );
            //MIL << _patternMap << endl;
            do {
              expandIncludes( unprocessed );
              expandExtending( unprocessed );
              _patternMap[unprocessed] = true;
              //MIL << _patternMap << endl;
            } while( (unprocessed = nextUnprocessed()) );
          }
        //SEC << "--- " << _patternMap.size() << " ----------------------------------" << endl;
        return _patternMap.size();
      }

      const PatternMap & patternMap() const
      { return _patternMap; }

    private:
      /** Get the next unprocessed Pattern \c _patternMap. */
      Pattern::constPtr nextUnprocessed() const
      {
        for ( PatternMap::const_iterator it = _patternMap.begin(); it != _patternMap.end(); ++it )
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
          expandInclude( Capability( it->c_str(), ResKind::pattern ) );
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
        Pattern::NameList c( pat_r->extends() );
        for_( it, c.begin(), c.end() )
        {
#warning TBD
          //expandIfExtends( pat_r, Capability( it->c_str(), ResKind::pattern ) );
        }
      }

      /** Store \c extending_r if it extends \c pat_r. */
      void expandIfExtends( const Pattern::constPtr & pat_r, const PoolItem & extending_r )
      {
        Pattern::constPtr extending( asKind<Pattern>(extending_r) );

        if ( ! extending->extends().empty() )
          {
#warning TBD
#if 0
            if ( std::find_if( extending->extends().begin(),
                               extending->extends().end(),
                               bind( &Impl::providedBy, this, pat_r, _1 ) )
                 != extending->extends().end() )
              {
                // an extends matches the Pattern
                _patternMap[extending];
                //DBG << mapEntry(*_patternMap.find(extending)) << endl;
              }
#endif
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
      ResPool    _pool;
      PatternMap _patternMap;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatternExpander
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PatternExpander::PatternExpander
    //	METHOD TYPE : Ctor
    //
    PatternExpander::PatternExpander( const ResPool & pool_r )
    : _pimpl( new Impl( pool_r ) )
    {}

    PatternExpander::size_type PatternExpander::expand( const Pattern::constPtr & pat_r )
    { return _pimpl->doExpand( pat_r ); }

    PatternExpander::size_type PatternExpander::size() const
    { return _pimpl->patternMap().size(); }

    bool PatternExpander::empty() const
    { return _pimpl->patternMap().empty(); }

    PatternExpander::const_iterator PatternExpander::begin() const
    { return make_map_key_begin( _pimpl->patternMap() ); }

    PatternExpander::const_iterator PatternExpander::end() const
    { return make_map_key_end( _pimpl->patternMap() ); }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
