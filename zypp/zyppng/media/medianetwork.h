/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPPNG_MEDIA_MEDIANETWORK_H_INCLUDED
#define ZYPPNG_MEDIA_MEDIANETWORK_H_INCLUDED

#include <zypp/media/MediaHandler.h>
#include <zypp/zyppng/core/Url>
#include <zypp/zyppng/media/network/AuthData>
#include <zypp/zyppng/media/network/TransferSettings>
#include <optional>

namespace zyppng {

class Download;
class Downloader;
class EventDispatcher;

class MediaHandlerNetwork : public zypp::media::MediaHandler
{
public:
  MediaHandlerNetwork(const Url &url_r, const zypp::Pathname &attach_point_hint_r);
  virtual ~MediaHandlerNetwork() override { try { release(); } catch(...) {} }

  TransferSettings & settings();

  // MediaHandler interface
protected:
  void attachTo(bool next) override;
  void releaseFrom(const std::string &ejectDev) override;
  void getFile(const zypp::filesystem::Pathname &filename, const zypp::ByteCount &expectedFileSize_r) const override;
  void getFiles(const std::vector<std::pair<zypp::filesystem::Pathname, zypp::ByteCount> > &files) const override;
  void getDir(const zypp::filesystem::Pathname &dirname, bool recurse_r) const override;
  void getDirInfo(std::list<std::string> &retlist, const zypp::filesystem::Pathname &dirname, bool dots) const override;
  void getDirInfo(zypp::filesystem::DirContent &retlist, const zypp::filesystem::Pathname &dirname, bool dots) const override;
  bool getDoesFileExist(const zypp::filesystem::Pathname &filename) const override;
  bool checkAttachPoint(const zypp::Pathname &apoint) const override;


  Url getFileUrl(const zypp::Pathname &filename_r) const;
  void handleRequestResult (const Download &req ) const;

private:
  void authenticate(const Download &, NetworkAuthData &auth, const std::string & availAuth) const;
  std::shared_ptr<Download> prepareRequest (Downloader &dlManager, const zypp::filesystem::Pathname &filename, const zypp::ByteCount &expectedFileSize_r = zypp::ByteCount() ) const;

private:
  TransferSettings _settings;
  std::optional<size_t> _prefetchCacheId;

  // MediaHandler interface
protected:
  void disconnectFrom() override;

  // MediaHandler interface
public:
  void precacheFiles(const std::vector<zypp::OnMediaLocation> &files) override;
};

}



#endif
