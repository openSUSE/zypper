#ifndef ZYPP_PARSER_YUM_REPOPARSEROPTS_H_
#define ZYPP_PARSER_YUM_REPOPARSEROPTS_H_

#include "zypp/base/DefaultIntegral.h"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /** YUM parser options */
  struct RepoParserOpts
  {
    /** Skip parsing of other.xml.gz */
    DefaultIntegral<bool,true> skipOther;

    /** Skip parsing of filelists.xml.gz */
    DefaultIntegral<bool,false> skipFilelists;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*ZYPP_PARSER_YUM_REPOPARSEROPTS_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
