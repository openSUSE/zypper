/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

File:       YUMtest.cc

Author:     Michael Radziej <mir@suse.de>
Maintainer: Michael Radziej <mir@suse.de>

Purpose:    main() to test the YUM parsers
/-*/

#include "zypp/parser/yum/YUMParser.h"
#include "zypp/base/Logger.h"

using namespace zypp;
using namespace zypp::parser;
using namespace zypp::parser::yum;
using namespace std;

namespace {
  void usage() {
    cerr << "YUMtest usage: "<< endl
    << "YUMtest TYPE" << endl
    << "TYPE: repomd|primary|group|pattern|filelist|other|patch|patches|product" << endl;
  }
}


int main(int argc, char **argv)
{
  if (argc < 2) {
    usage();
    return 2;
  }

//  set_log_filename("-");

  try {
    if (!strcmp(argv[1],"repomd")) {
      YUMRepomdParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             cout << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if (!strcmp(argv[1],"primary")) {
      YUMPrimaryParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             cout << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if (!strcmp(argv[1],"group")) {
      YUMGroupParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             cout << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if (!strcmp(argv[1],"pattern")) {
      YUMPatternParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             cout << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if (!strcmp(argv[1],"filelist")) {
      YUMFileListParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             cout << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if (!strcmp(argv[1],"other")) {
      YUMOtherParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             cout << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if (!strcmp(argv[1],"patch")) {
      YUMPatchParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             cout << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if (!strcmp(argv[1],"patches")) {
      YUMPatchesParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             cout << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if (!strcmp(argv[1],"product")) {
      YUMProductParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             cout << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else {
      usage();
      return 2;
    }
  }  
  catch (XMLParserError& err) {
    cerr << "** ouch **" << endl
      << "syntax error encountered in XML input:" << endl
      << err.msg() << " " << err.position() << endl;
    return 1;
  }
  
           return 0;
}
