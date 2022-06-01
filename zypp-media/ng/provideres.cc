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
#include "private/provideres_p.h"

namespace zyppng {

  ProvideRes::ProvideRes( std::shared_ptr<ProvideResourceData> dataPtr ) : _data(dataPtr)
  { }

  ProvideRes::~ProvideRes()
  { }

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

  const zypp::Url &ProvideRes::resourceUrl () const
  {
    return _data->_resourceUrl;
  }

  const HeaderValueMap &ProvideRes::headers () const
  {
    return _data->_responseHeaders;
  }

}
