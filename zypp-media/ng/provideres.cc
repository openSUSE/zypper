/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "provideres.h"
#include "private/provide_p.h"
#include "private/providequeue_p.h"

namespace zyppng {

  ProvideRes::ProvideRes( std::shared_ptr<ProvideResourceData> dataPtr ) : _data(dataPtr)
  { }

  ProvideRes::~ProvideRes()
  {
    _data->_lastUnref = std::chrono::system_clock::now();
  }

  const zypp::filesystem::Pathname ProvideRes::file() const
  {
    return _data->_myFile;
  }

  const zypp::ManagedFile & ProvideRes::asManagedFile () const
  {
    return _data->_myFile;
  }

  const ProvideMediaHandle &ProvideRes::mediaHandle () const
  {
    return _data->_mediaHandle;
  }

}
