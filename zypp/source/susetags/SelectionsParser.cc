/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SelectionsParser.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/source/susetags/SelectionsParser.h"
#include "zypp/parser/tagfile/Parser.h"
#include "zypp/Selection.h"
#include "zypp/detail/SelectionImpl.h"
#include "zypp/CapFactory.h"
#include "zypp/CapSet.h"


using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      using namespace parser::tagfile;

      struct SelectionsParser : public parser::tagfile::Parser
      {
        std::list<Selection::Ptr> result;

        shared_ptr<detail::SelectionImpl> pkgImpl;
        NVRAD nvrad;

        bool pkgPending() const
        { return pkgImpl; }

        void collectPkg( const shared_ptr<detail::SelectionImpl> & nextPkg_r
                         = shared_ptr<detail::SelectionImpl>() )
        {
          if ( pkgPending() )
            {
              result.push_back( detail::makeResolvableFromImpl( nvrad, pkgImpl ) );
            }
          pkgImpl = nextPkg_r;
        }

        void newPkg()
        {
          collectPkg( shared_ptr<detail::SelectionImpl>(new detail::SelectionImpl) );
        }

        void collectDeps( const std::list<std::string> & depstr_r, CapSet & capset )
        {
          for ( std::list<std::string>::const_iterator it = depstr_r.begin();
                it != depstr_r.end(); ++it )
            {
              capset.insert( CapFactory().parse( ResTraits<Selection>::kind, *it ) );
            }
        }

        /* Consume SingleTag data. */
        virtual void consume( const STag & stag_r )
        {
          if ( stag_r.stag.isPlain( "Pkg" ) )
            {
              std::vector<std::string> words;
              str::split( stag_r.value, std::back_inserter(words) );

              if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
                ZYPP_THROW( ParseException( "Pkg" ) );

              newPkg();
              nvrad = NVRAD( words[0], Edition(words[1],words[2]), Arch(words[3]) );
            }
        }

        /* Consume MulitTag data. */
        virtual void consume( const MTag & mtag_r )
        {
          if ( ! pkgPending() )
            return;

          if ( mtag_r.stag.isPlain( "Prv" ) )
            {
              collectDeps( mtag_r.value, nvrad[Dep::PROVIDES] );
            }
          else if ( mtag_r.stag.isPlain( "Prq" ) )
            {
              collectDeps( mtag_r.value, nvrad[Dep::PREREQUIRES] );
            }
          else if ( mtag_r.stag.isPlain( "Req" ) )
            {
              collectDeps( mtag_r.value, nvrad[Dep::REQUIRES] );
            }
          else if ( mtag_r.stag.isPlain( "Con" ) )
            {
              collectDeps( mtag_r.value, nvrad[Dep::CONFLICTS] );
            }
          else if ( mtag_r.stag.isPlain( "Obs" ) )
            {
              collectDeps( mtag_r.value, nvrad[Dep::OBSOLETES] );
            }
        }

        virtual void parseEnd()
        { collectPkg(); }
      };

      ////////////////////////////////////////////////////////////////////////////

      std::list<Selection::Ptr> parseSelections( const Pathname & file_r )
      {
        SelectionsParser p;
        p.parse( file_r );
        return p.result;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
