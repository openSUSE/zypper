/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_SOURCES_H
#define ZMART_SOURCES_H

#include "zypp/Url.h"

//! calls init_system_sources if not disabled by user (or non-root)
void cond_init_system_sources();
void init_system_sources();
void include_source_by_url( const zypp::Url &url );
bool parse_repo_file (const std::string& file, std::string& url, std::string& alias);
void add_source_by_url( const zypp::Url &url, std::string alias );
void remove_source( const std::string& anystring );
void rename_source( const std::string& anystring, const std::string& newalias );
void list_system_sources();
void refresh_sources();
void warn_if_zmd ();

//! download a copy of a remote file or just return the argument
// The file is deleted when this class is destroyed
class MediaWrapper : private zypp::base::NonCopyable {
public:
  MediaWrapper (const std::string& filename_or_url);
  ~MediaWrapper ();
  std::string localPath () const {
    return _local_path;
  }

private:
  zypp::media::MediaManager _mm; // noncopyable
  zypp::media::MediaId _id;  
  std::string _local_path;
};

#endif

