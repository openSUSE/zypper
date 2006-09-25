/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_H
#define ZMART_H

#include <zypp/base/LogControl.h>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/Locale.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/zypp_detail/ZYppReadOnlyHack.h>
#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>
#include <zypp/ResStore.h>
#include <zypp/base/String.h>
#include <zypp/Digest.h>
#include <zypp/CapFactory.h>

#define ZYPP_CHECKPATCHES_LOG "/var/log/zypp-zmart.log"
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::zmart"
#define RANDOM_TOKEN "sad987432JJDJD948394DDDxxx22"

struct Settings
{
  Settings()
  : previous_token(RANDOM_TOKEN),
  verbose(0),  
  previous_code(-1),
  disable_system_sources(false),
  disable_system_resolvables(false)
  {}

  std::list<zypp::Url> additional_sources;
  std::string previous_token;
  int verbose;
  int previous_code;
  std::string command;
  bool disable_system_sources;
  bool disable_system_resolvables;
};

struct RuntimeData
{
  RuntimeData()
  : patches_count(0),
  security_patches_count(0)
  {}
    
  std::list<zypp::Source_Ref> sources;
  int patches_count;
  int security_patches_count;
  std::vector<std::string> packages_to_install; 
};

extern RuntimeData gData;
extern Settings gSettings;
extern std::ostream no_stream;

#define COND_STREAM(STREAM,LEVEL) ((gSettings.verbose >= LEVEL)? STREAM: no_stream)
#define cerr_v COND_STREAM(cerr,1)
#define cout_v COND_STREAM(cout,1)
#define cerr_vv COND_STREAM(cerr,2)
#define cout_vv COND_STREAM(cout,2)

#endif

