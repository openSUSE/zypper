#ifndef ZMART_SOURCES_H
#define ZMART_SOURCES_H

#include "zypp/Url.h"

/**
 * Reads known enabled repositories and stores them in gData.
 * This command also refreshes repos with auto-refresh enabled.
 */
void init_repos();

/**
 * 
 */
void list_repos();

/**
 * 
 */
void refresh_repos();



bool parse_repo_file (const std::string& file, std::string& url, std::string& alias);

void add_source_by_url( const zypp::Url &url, const std::string &alias,
                        const std::string &type = "",
                        bool enabled = true, bool refresh = true );

/**
 * If ZMD process found, notify user that ZMD is running and that changes
 * to repositories will not be synchronized with it. To be used with commands
 * manipulating repositories like <tt>addrepo</tt> or <tt>rmrepo</tt>. 
 */
void warn_if_zmd();

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

//! calls init_system_sources if not disabled by user (or non-root)
void cond_init_system_sources(); // OLD
void init_system_sources(); // OLD
void remove_source( const std::string& anystring ); // OLD
void rename_source( const std::string& anystring, const std::string& newalias ); // OLD
void include_source_by_url( const zypp::Url &url ); // OLD

#endif
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
