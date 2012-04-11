/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/AttrMatcher.h
 *
*/
#ifndef ZYPP_SAT_ATTRMATCHER_H
#define ZYPP_SAT_ATTRMATCHER_H
#warning sat::AttrMatcher was renamed to StrMatcher. Deprecated include of zypp/sat/AttrMatcher.h use zypp/base/StrMatcher.h

#include "zypp/base/StrMatcher.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    typedef StrMatcher AttrMatcher;

  } // namespace sat
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_ATTRMATCHER_H
