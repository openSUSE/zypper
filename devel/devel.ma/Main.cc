#include <iostream>
#include <ctime>

#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>

#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>

#include <zypp/PathInfo.h>
#include <zypp/SourceFactory.h>
#include <zypp/source/Builtin.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include <zypp/ResFilters.h>

using namespace std;
using namespace zypp;

///////////////////////////////////////////////////////////////////

bool all( const ResObject::Ptr & )
{ return true; }

bool doPrintM( const ResObject::Ptr & p )
{
  MIL << *p << endl;
  return true;
}
bool doPrint( const ResObject::Ptr & p )
{
  DBG << *p << endl;
  return true;
}

struct Print
{
  Print( const std::string & name_r )
  : _name( name_r )
  {}
  std::string _name;
  bool operator()( ResObject::Ptr p ) const
  {
    DBG << _name << endl;
    return true;
  }
};


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  Pathname f( "./p" );
  Source src( SourceFactory().createFrom( new source::SuseTagsImpl( f ) ) );
  INT << src.resolvables().size() << endl;

  src.resolvables().forEach( chain( resfilter::ByKind( ResTraits<Package>::kind ),
                    resfilter::ByName( "rpm" )
                  ),
             resfilter::chain( Print("1"),
                    chain( resfilter::chain( Print("A"), resfilter::chain( Print("B"), Print("C") ) ),
                           Print("2")
                         )
                  )
           );



  INT << "===[END]============================================" << endl;
  return 0;
}
