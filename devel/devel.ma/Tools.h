#ifndef Tools_h
#define Tools_h

#include <iostream>

#include "Measure.h"
#include "Printing.h"

#include <zypp/ResObject.h>

#include <zypp/SourceFactory.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

using namespace zypp;
using std::endl;

///////////////////////////////////////////////////////////////////
// rstats
template<class _IntT>
  struct Counter
  {
    Counter()                : _value( _IntT(0) )         {}
    Counter( _IntT value_r ) : _value( _IntT( value_r ) ) {}
    operator       _IntT &()       { return _value; }
    operator const _IntT &() const { return _value; }

    _IntT _value;
  };

struct Rstats : public std::unary_function<ResObject::constPtr, void>
{
  void operator()( ResObject::constPtr ptr )
  {
    ++_total;
    ++_perKind[ptr->kind()];
  }

  typedef std::map<ResolvableTraits::KindType,Counter<unsigned> > KindMap;
  Counter<unsigned> _total;
  KindMap           _perKind;
};

std::ostream & operator<<( std::ostream & str, const Rstats & obj )
{
  str << "Total: " << obj._total;
  for( Rstats::KindMap::const_iterator it = obj._perKind.begin(); it != obj._perKind.end(); ++it )
    {
      str << endl << "  " << it->first << ":\t" << it->second;
    }
  return str;
}

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
      ret = SourceFactory().createFrom( url_r );
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
