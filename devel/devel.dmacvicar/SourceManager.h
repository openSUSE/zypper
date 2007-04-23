
#ifndef ZYPP_NEW_SOURCEMANAGER_H
#define ZYPP_NEW_SOURCEMANAGER_H

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/Pathname.h"

namespace zypp2
{
  
  class SourceManager : public base::ReferenceCounted, private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const SourceManager & obj );
  };
  
  
}
#endif