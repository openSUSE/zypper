/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_SUMMARY_H_
#define ZYPPER_UTILS_SUMMARY_H_

#include <set>
#include <map>
#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/ByteCount.h"
#include "zypp/ResObject.h"
#include "zypp/ResPool.h"


class Summary : private zypp::base::NonCopyable
{
public:
  typedef std::pair<zypp::ResObject::constPtr, zypp::ResObject::constPtr> ResPair;
  struct ResPairNameCompare
  {
    inline bool operator()(const ResPair & p1, const ResPair & p2) const;
  };
  typedef std::set<ResPair, ResPairNameCompare> ResPairSet;
  typedef std::map<zypp::ResKind, ResPairSet> KindToResPairSet;

  enum _view_options
  {
    DEFAULT                 = 0x0300,
    DETAILS                 = 0x00ff,

    SHOW_VERSION            = 0x0001,
    SHOW_ARCH               = 0x0002,
    SHOW_REPO               = 0x0004,
    SHOW_VENDOR             = 0x0008,

    SHOW_SUGGESTED          = 0x0100,
    SHOW_RECOMMENDED        = 0x0200,
    SHOW_UNSUPPORTED        = 0x0400,

    SHOW_ALL                = 0xffff
  };

  typedef enum _view_options ViewOptions;

public:
  Summary(const zypp::ResPool & pool, const ViewOptions options = DEFAULT);
  ~Summary() {}

  void setViewOptions(const ViewOptions options)
  { _viewop = options; }
  ViewOptions viewOptions() const
  { return _viewop; }
  void setViewOption(const ViewOptions option)
  { _viewop = (ViewOptions) (_viewop | option); }
  void unsetViewOption(const ViewOptions option)
  { _viewop = (ViewOptions) (_viewop & ~option); }
  void toggleViewOption(const ViewOptions option)
  { _viewop & option ? unsetViewOption(option) : setViewOption(option); }
  void setForceNoColor(bool value = true)
  { _force_no_color = value; }

  void writeNewlyInstalled(std::ostream & out);
  void writeRemoved(std::ostream & out);
  void writeUpgraded(std::ostream & out);
  void writeDowngraded(std::ostream & out);
  void writeReinstalled(std::ostream & out);
  void writeRecommended(std::ostream & out);
  void writeSuggested(std::ostream & out);
  void writeChangedArch(std::ostream & out);
  void writeChangedVendor(std::ostream & out);
  void writeUnsupported(std::ostream & out);
  void writePackageCounts(std::ostream & out);
  void writeDownloadAndInstalledSizeSummary(std::ostream & out);


  unsigned packagesToGetAndInstall() const
  { return _inst_pkg_total; }
  unsigned packagesToRemove() const;
  unsigned packagesToUpgrade() const;
  unsigned packagesToDowngrade() const;
  const zypp::ByteCount & toDownload() const
  { return _todownload; }
  const zypp::ByteCount & installedSizeChange() const
  { return _inst_size_change; }

  bool needMachineReboot() const
  { return _need_reboot; }

  bool needPkgMgrRestart() const
  { return _need_restart; }


  void dumpTo(std::ostream & out);
  void dumpAsXmlTo(std::ostream & out);

private:
  void readPool(const zypp::ResPool & pool);
  void writeResolvableList(std::ostream & out, const ResPairSet & resolvables);
  void writeXmlResolvableList(std::ostream & out, const KindToResPairSet & resolvables);

private:
  ViewOptions _viewop;
  mutable unsigned _wrap_width;
  bool _force_no_color;

  bool _need_reboot;
  bool _need_restart;

  zypp::ByteCount _todownload;
  zypp::ByteCount _inst_size_change;

  // STATS

  unsigned _inst_pkg_total;

  KindToResPairSet toinstall;
  KindToResPairSet toupgrade;
  KindToResPairSet todowngrade;
  KindToResPairSet toreinstall;
  KindToResPairSet toremove;
  KindToResPairSet tochangearch;
  KindToResPairSet tochangevendor;
  /** objects from previous lists that are not supported */
  KindToResPairSet unsupported;
};

#endif /* ZYPPER_UTILS_SUMMARY_H_ */
