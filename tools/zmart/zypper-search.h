/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#ifndef ZYPPERSEARCH_H_
#define ZYPPERSEARCH_H_

#include <string>
#include <boost/regex.hpp>

#include "zypper-tabulator.h"

using std::string;
using namespace boost;

/**
 * @brief: search options
 */
class ZyppSearchOptions {
public:
  ZyppSearchOptions () :
    _installed_only(false), _uninstalled_only(false), _match_all(true) {}
  
  bool installedOnly() const { return _installed_only; }
  bool uninstalledOnly() const { return _uninstalled_only; }
  bool matchAll() const { return _match_all; }
  bool matchAny() const { return !_match_all; }

  void setInstalledOnly(bool const installed_only = true) {
    _installed_only = installed_only;
  }
  void setUnInstalledOnly(bool const uninstalled_only = true) {
    _uninstalled_only = uninstalled_only;
  }
  void setMatchAll(bool const match_all = true) { _match_all = match_all; }
  void setMatchAny(bool const match_any = true) { _match_all = !match_any; }

private:
  bool _installed_only;
  bool _uninstalled_only;
  bool _match_all;
};

/**
 *
 */
class ZyppSearch {
public:
  ZyppSearch (ZyppSearchOptions const &options);
  ZyppSearch (ZyppSearchOptions const &options, vector<string> const &qstrings);
  Table doSearch();

private:
  ZyppSearchOptions const &_options;
  vector<string> const &_qstrings;
  vector<string> const empty_vector; // default initializer for _qstrings
  regex _reg;

  bool init() const;
  void setupRegexp();
};

#endif /*ZYPPERSEARCH_H_*/
