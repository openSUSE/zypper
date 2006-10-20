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
#include <boost/function.hpp>
#include <zypp/ZYpp.h>

/**
 * @brief: search options
 */
class ZyppSearchOptions {
public:
  enum InsFilter {
    ALL,
    INSTALLED_ONLY,
    UNINSTALLED_ONLY
  };

  ZyppSearchOptions () :
    _ifilter(ALL),
    _match_all(true), _match_words(false), _match_exact(false),
    _search_descriptions(false), _case_sensitive(false),
    _kind(zypp::Resolvable::Kind())
    {}

  void resolveConflicts();

  InsFilter installedFilter() const { return _ifilter; }
  bool matchAll() const { return _match_all; }
  bool matchAny() const { return !_match_all; }
  bool matchWords() const { return _match_words; }
  bool matchExact() const { return _match_exact; }
  bool searchDescriptions() const { return _search_descriptions; }
  bool caseSensitive() const { return _case_sensitive; }
  zypp::Resolvable::Kind kind() const { return _kind; }

  void setInstalledFilter(const InsFilter ifilter) { _ifilter =  ifilter; }
  void setMatchAll(const bool match_all = true) { _match_all = match_all; }
  void setMatchAny(const bool match_any = true) { _match_all = !match_any; }
  void setMatchWords(const bool match_words = true) { _match_words = match_words; }
  void setMatchExact(const bool match_exact = true) { _match_exact = match_exact; }
  void setSearchDescriptions(const bool search_descriptions = true) { _search_descriptions = search_descriptions; }
  void setCaseSensitive(const bool case_sensitive = true) { _case_sensitive = case_sensitive; }
  void setKind(const zypp::Resolvable::Kind & kind) { _kind = kind; }

private:
  InsFilter _ifilter;
  bool _match_all;
  bool _match_words;
  bool _match_exact;
  bool _search_descriptions;
  bool _case_sensitive;
  zypp::Resolvable::Kind _kind;
};

/**
 *
 */
class ZyppSearch {
public:
  ZyppSearch (const ZyppSearchOptions & options, const vector<std::string> & qstrings = vector<string>());
  void doSearch(const boost::function<void(const zypp::PoolItem &)> & f);

private:
  const ZyppSearchOptions & _options;
  const vector<std::string> & _qstrings;
  boost::regex _reg;

  bool init() const;
  void setupRegexp();
  string wildcards2regex(const string & str) const;
  bool match(const zypp::PoolItem & pool_item);
  TableRow createRow(const zypp::PoolItem & pool_item);
};

#endif /*ZYPPERSEARCH_H_*/
