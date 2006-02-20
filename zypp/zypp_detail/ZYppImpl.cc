/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/zypp_detail/ZYppImpl.cc
 *
*/

#include <sys/utsname.h>
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/zypp_detail/ZYppImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace zypp_detail
  { /////////////////////////////////////////////////////////////////

    inline Locale defaultTextLocale()
    {
      Locale ret( "en" );
      char * envlist[] = { "LC_ALL", "LC_CTYPE", "LANG", NULL };
      for ( char ** envvar = envlist; *envvar; ++envvar )
        {
          char * envlang = getenv( *envvar );
          if ( envlang )
            {
              std::string envstr( envlang );
              if ( envstr != "POSIX" && envstr != "C" )
                {
                  Locale lang( envlang );
                  if ( lang != Locale::noCode )
                    {
                      ret = lang;
                      break;
                    }
                }
            }
        }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZYppImpl::ZYppImpl
    //	METHOD TYPE : Constructor
    //
    ZYppImpl::ZYppImpl()
    : _textLocale( defaultTextLocale() )
    , _pool()
    , _sourceFeed( _pool )
    , _resolver( new Resolver(_pool.accessor()) )
    {
      MIL << "defaultTextLocale: '" << _textLocale << "'" << endl;

      struct utsname buf;
      if (uname (&buf) < 0) {
	ERR << "Can't determine system architecture" << endl;
      }
      else {
	MIL << "System architecture is '" << buf.machine << "'" << endl;
	_architecture = Arch(buf.machine);
      }

    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZYppImpl::~ZYppImpl
    //	METHOD TYPE : Destructor
    //
    ZYppImpl::~ZYppImpl()
    {
    }

    void ZYppImpl::addResolvables (const ResStore& store, bool installed)
    {
	_pool.insert(store.begin(), store.end(), installed);
    }

    void ZYppImpl::removeResolvables (const ResStore& store)
    {
        for (ResStore::iterator it = store.begin(); it != store.end(); it++)
	{
    	    _pool.erase(*it);
	}
    }

    Target_Ptr ZYppImpl::target() const
    {
      if (! _target)
	ZYPP_THROW(Exception("Target not initialized."));
      return _target;
     }

    void ZYppImpl::initTarget(const Pathname & root, bool commit_only)
    {
      if (_target)
	_target = Target_Ptr();
      _target = new Target(root);
      if (!commit_only)
      {
	_target->enableStorage(root);
	addResolvables (_target->resolvables(), true);
      }
    }

    void ZYppImpl::finishTarget()
    {
#warning not removing target resolvables for now
//      if (_target)
//	removeResolvables (_target->resolvables());
      _target = 0;
    }

    /** \todo Remove workflow from target, lot's of it could be done here,
    * and target used for transact. */
    ZYpp::CommitResult ZYppImpl::commit( int medianr_r )
    {
      MIL << "Attempt to commit (medianr " << medianr_r << ")" << endl;
      if (! _target)
	ZYPP_THROW( Exception("Target not initialized.") );

      ZYpp::CommitResult res;
      // must redirect to Target::Impl. This kind of commit should not be
      // in the Target interface.
      res._result = _target->commit( pool(), medianr_r,
                                     res._errors, res._remaining, res._srcremaining );

      MIL << "Commit (medianr " << medianr_r << ") returned: "
          << res._result
          << " (errors " << res._errors.size()
          << ", remaining " << res._remaining.size()
          << ", srcremaining " << res._srcremaining.size()
          << ")" << endl;
      return res;
    }


    void ZYppImpl::setArchitecture( const Arch & arch )
    {
	_architecture = arch;
	if (_resolver) _resolver->setArchitecture( arch );
    }

    Pathname ZYppImpl::homePath() const
    { return _home_path.empty() ? Pathname("/var/lib/zypp") : _home_path; }
    void ZYppImpl::setHomePath( const Pathname & path )
    { _home_path = path; }  
    
    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const ZYppImpl & obj )
    {
      return str << "ZYppImpl";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace zypp_detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
