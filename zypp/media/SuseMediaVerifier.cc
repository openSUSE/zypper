/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/parser/media/SuseMediaVerifier.cc
 *
*/

#include "zypp/media/SuseMediaVerifier.h"
#include "zypp/media/MediaManager.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/Pathname.h"

#include <string>
#include <list>

namespace zypp {

  namespace media {
  
    SuseMediaVerifier::SuseMediaVerifier() : MediaVerifierBase() {}

    SuseMediaVerifier::~SuseMediaVerifier() {}

    bool SuseMediaVerifier::isDesiredMedia(const MediaAccessRef &ref, MediaNr mediaNr)
    {
	DBG << "isDesiredMedia(): for media nr " << mediaNr << std::endl;

	if( !ref)
    	    DBG << "isDesiredMedia(): invalid media handle" << std::endl;

	std::list<std::string> lst;
    
	Pathname dir("/media." + str::numstring(mediaNr));

	DBG << "isDesiredMedia(): checking " << dir.asString() << std::endl;

	try
	{
    	    if( ref )
    		ref->dirInfo(lst, dir, false);
	}
	catch( ... )
	{}

	DBG << "isDesiredMedia(): media "
    		<< (lst.empty() ? "does not contain" : "contains")
    		<< " the " << dir.asString() << " directory."
    		<< std::endl;

	return !lst.empty();
    }

  }
  
}

