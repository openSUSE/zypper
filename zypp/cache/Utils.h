/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_DATA_UTILS_H
#define ZYPP_DATA_UTILS_H

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/CheckSum.h"
#include <boost/logic/tribool.hpp>

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////

	int tribool_to_int( boost::tribool b );
	boost::tribool int_to_tribool( int i );
	std::string checksum_to_string( const CheckSum &checksum );
	CheckSum string_to_checksum( const std::string &checksum );
  
}
}

#endif