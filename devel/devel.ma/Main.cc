#include <iostream>

#include "Tools.h"

#include "zypp/Bit.h"

using namespace std;
using namespace zypp;
#if 0
namespace zypp
{

  struct RpmFlagsBase
  {
    typedef uint16_t                 FieldType;
    typedef bit::BitField<FieldType> BitFieldType;

    enum FlagType
      {
        ignore,
        install,
        erase
      };
    enum FlagValue
      {
        none
        //
        --test
        --justdb
        --nodeps
        --noscripts
        --notriggers
        --repackage
        --force
        //
        // install
        // upgrade
        -nodigest
        -nosignature
        -nosuggest
        -noorder
        -replacefiles
        -replacepkgs
        -oldpackage
        -excludedocs
        -ignoresize
        -ignorearch
        -ignoreos
        //


      };


  };


}
#endif
/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  //ResStatus stat;
  //MIL << stat << endl;

  std::string line = " a b c ";
  std::vector<std::string> words;
  DBG << str::split( line, std::back_inserter(words), " " ) << " " << words.size() << endl;
  DBG << str::split( line, std::back_inserter(words), " " ) << " " << words.size() << endl;



  INT << "===[END]============================================" << endl << endl;
  return 0;
}

