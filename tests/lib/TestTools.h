#ifndef TESTTOOLS_H
#define TESTTOOLS_H

#include <iostream>
#include <fstream>

#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"

namespace TestTools {
  //read all contents of a file into a string)
  std::string readFile ( const zypp::Pathname &file )
  {
    if ( ! zypp::PathInfo( file ).isFile() ) {
      return std::string();
    }
    std::ifstream istr( file.asString().c_str() );
    if ( ! istr ) {
      return std::string();
    }

    std::string str((std::istreambuf_iterator<char>(istr)),
      std::istreambuf_iterator<char>());
    return str;
  }
}


#endif // TESTTOOLS_H
