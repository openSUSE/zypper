/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/FileReaderBase.h
 * 
 */
#ifndef ZYPP_PARSER_YUM_FILEREADERBASE_H_
#define ZYPP_PARSER_YUM_FILEREADERBASE_H_

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Base for yum::*FileReaders. Implements parsing methods for common metadata.
   */
  class FileReaderBase : private base::NonCopyable
  {
  protected:
    class BaseImpl;
  };


    } // ns yum
  } // ns parser
} // ns zypp


#endif /*ZYPP_PARSER_YUM_FILEREADERBASE_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
