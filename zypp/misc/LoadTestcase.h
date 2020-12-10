/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/misc/LoadTestcase.h
 *
*/
#ifndef ZYPP_MISC_LOADTESTCASE_H
#define ZYPP_MISC_LOADTESTCASE_H

#include <zypp/Pathname.h>
#include <zypp/Url.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/misc/TestcaseSetup.h>

#include <optional>
#include <memory>

namespace zypp::misc::testcase {

  struct TestcaseTrial
  {
    struct Node {
      struct Impl;

      Node();
      ~Node();
      const std::string &name  () const;
      std::string &name  ();
      const std::string &value () const;
      std::string &value ();

      const std::string &getProp( const std::string &name, const std::string &def = std::string() ) const;
      const std::map<std::string, std::string> &properties() const;
      std::map<std::string, std::string> &properties();
      const std::vector<std::shared_ptr<Node>> &children() const;
      std::vector<std::shared_ptr<Node>> &children();

    private:
      RWCOW_pointer<Impl> _pimpl;

    };

    TestcaseTrial();
    ~TestcaseTrial();
    const std::vector<Node> &nodes () const;
    std::vector<Node> &nodes ();
  private:
    struct Impl;
    RWCOW_pointer<Impl> _pimpl;
  };

  class LoadTestcase : private zypp::base::NonCopyable
  {
  public:
    struct Impl;
    using TestcaseTrials = std::vector<TestcaseTrial>;

    enum Type {
      None,
      Helix,
      Yaml
    };

    LoadTestcase();
    ~LoadTestcase();

    bool loadTestcaseAt ( const zypp::Pathname &path, std::string *err );
    static Type testcaseTypeAt ( const zypp::Pathname &path );

    const TestcaseSetup &setupInfo() const;
    const TestcaseTrials &trialInfo() const;

  private:
    std::unique_ptr<Impl> _pimpl;
  };

}


#endif // ZYPP_MISC_LOADTESTCASE_H
