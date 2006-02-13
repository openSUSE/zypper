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
	addResolvables (_target->resolvables(), true);
    }

    void ZYppImpl::finishTarget()
    {
      if (_target)
	removeResolvables (_target->resolvables());
      _target = 0;
    }

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
