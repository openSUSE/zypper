/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/data/RecordId.h
 *
*/
#ifndef ZYPP_DATA_RECORDID_H
#define ZYPP_DATA_RECORDID_H

#include "zypp/base/DefaultIntegral.h"

namespace zypp
{
  namespace data
  {
    /** Cache store record id. */
    typedef DefaultIntegral<long long, -1> RecordId;
    /** The default RecordId is a value we don't use for records. */
    static const RecordId noRecordId;
  }
}

#endif
