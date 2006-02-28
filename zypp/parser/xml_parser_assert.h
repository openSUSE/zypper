/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml_parser_assert.h
 *
*/


#ifndef xml_parser_assert_h
#define xml_parser_assert_h

#include "zypp/base/Exception.h"

#define xml_assert(ptr)			\
  if (! ptr)				\
    ZYPP_THROW(Exception("XML parser error: null pointer check failed"))


#endif
