/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTIL_AUGEAS_H_
#define ZYPPER_UTIL_AUGEAS_H_

#include <string>

extern "C"
{
  #include <augeas.h>
}

#include "zypp/base/NonCopyable.h"
#include "zypp/TriBool.h"

/**
 * Zypper's wrapper around Augeas.
 */
class Augeas : private zypp::base::NonCopyable
{
public:
  Augeas(const std::string & file = "");
  ~Augeas();

  std::string get(const std::string & augpath) const;

  std::string getOption(const std::string & option) const;
  zypp::TriBool isCommented(const std::string & option, bool global) const;
  void comment(const std::string & option);
  void uncomment(const std::string & option);

  ::augeas * getAugPtr()
  { return _augeas; }

private:
  std::string userOptionPath(
      const std::string & section, const std::string & option) const;

  zypp::TriBool isCommented(const std::string & section,
      const std::string & option,
      bool global) const;

private:
  ::augeas * _augeas;
  std::string _homedir;
  /**
   * Path of the config file in the augeas tree,
   * e.g. /files/path/to/user/zypper.conf
   */
  std::string _user_conf_path;
  bool _got_global_zypper_conf;
  bool _got_user_zypper_conf;

  mutable int _last_get_result;
};

#endif /* ZYPPER_UTIL_AUGEAS_H_ */
