#ifndef Tools_h
#define Tools_h

#include <iostream>

#include "Measure.h"
#include "Printing.h"

#include <zypp/base/Counter.h>

#include <zypp/Date.h>
#include <zypp/ResObject.h>
#include <zypp/pool/PoolStats.h>

#include <zypp/SourceFactory.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

using namespace zypp;
using std::endl;

///////////////////////////////////////////////////////////////////
// rstats

typedef zypp::pool::PoolStats Rstats;

template<class _Iterator>
  void rstats( _Iterator begin, _Iterator end )
  {
    DBG << __PRETTY_FUNCTION__ << endl;
    Rstats stats;
    for_each( begin, end, functor::functorRef<void,ResObject::constPtr>(stats) );
    MIL << stats << endl;
  }

template<class _Container>
  void rstats( const _Container & c )
  {
    rstats( c.begin(), c.end() );
  }

///////////////////////////////////////////////////////////////////
inline Source_Ref createSource( const Url & url_r )
{
  Source_Ref ret;
  Measure x( "createSource: " + url_r.asString() );
  try
    {
      ret = SourceFactory().createFrom( url_r, "/", Date::now().asSeconds() );
    }
  catch ( const Exception & )
    {
      return Source_Ref::noSource;
    }
  x.start( "parseSource: " + url_r.asString() );
  ret.resolvables();
  x.stop();
  MIL << "Content " << ret << "{" << endl;
  rstats( ret.resolvables() );
  MIL << "}" << endl;

  return ret;
}
inline Source_Ref createSource( const std::string & url_r )
{
  try
    {
      return createSource( Url(url_r) );
    }
  catch ( const Exception & )
    {
      return Source_Ref::noSource;
    }
}

#endif // Tools_h
