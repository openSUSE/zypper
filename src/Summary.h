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

#include <zypp/base/PtrTypes.h>
#include <zypp/ByteCount.h>
#include <zypp/base/DefaultIntegral.h>
#include <zypp/ResObject.h>
#include <zypp/ResPool.h>


class Summary : private base::NonCopyable
{
public:
  /**
   * \brief ResPair is used to track removed and installed ResObjects.
   *        usually only 'second' is intialized, the 'first' var is only
   *        required to track upgrades, first is the removed and second the installed package
   */
  typedef std::pair<ResObject::constPtr, ResObject::constPtr> ResPair;
  struct ResPairNameCompare
  {
    inline bool operator()( const ResPair & p1, const ResPair & p2 ) const;
  };
  typedef std::set<ResPair, ResPairNameCompare> ResPairSet;
  typedef std::map<ResKind, ResPairSet> KindToResPairSet;

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
    SHOW_NOT_UPDATED        = 0x0800,
    SHOW_LOCKS              = 0x1000,

    UPDATESTACK_ONLY        = 0x2000,  //< required for zypper patch
    PATCH_REBOOT_RULES      = 0x4000,  // for zypper patch bsc#1183268

    SHOW_ALL                = 0xffff
  };

  typedef enum _view_options ViewOptions;

public:
  Summary( const ResPool & pool, const ViewOptions options = DEFAULT );
  ~Summary() {}

  void setViewOptions( const ViewOptions options )	{ _viewop = options; }
  ViewOptions viewOptions() const			{ return _viewop; }
  void setViewOption( const ViewOptions option )	{ _viewop = (ViewOptions) (_viewop | option); }
  void unsetViewOption( const ViewOptions option )	{ _viewop = (ViewOptions) (_viewop & ~option); }
  void toggleViewOption( const ViewOptions option )	{ _viewop & option ? unsetViewOption(option) : setViewOption(option); }
  bool hasViewOption( const ViewOptions option ) const  { return _viewop & option; }
  void setForceNoColor( bool value = true )		{ _force_no_color = value; }
  void setDownloadOnly( bool value = true )		{ _download_only = value; }

  void writeNewlyInstalled( std::ostream & out );
  void writeRemoved( std::ostream & out );
  void writeUpgraded( std::ostream & out );
  void writeDowngraded( std::ostream & out );
  void writeReinstalled( std::ostream & out );
  void writeRecommended( std::ostream & out );
  void writeSuggested( std::ostream & out );
  void writeChangedArch( std::ostream & out );
  void writeChangedVendor( std::ostream & out );
  void writeSupportUnknown( std::ostream & out );
  void writeSupportUnsupported( std::ostream & out );
  void writeSupportNeedACC( std::ostream & out );
  void writeNotUpdated( std::ostream & out );
  void writePackageCounts( std::ostream & out );
  void writeDownloadAndInstalledSizeSummary( std::ostream & out );
  void writeLocked( std::ostream & out );
  void writeRebootNeeded( std::ostream & out );

  unsigned packagesToGetAndInstall() const		{ return _inst_pkg_total; }

  unsigned packagesToInstall() const;
  unsigned packagesToUpgrade() const;
  unsigned packagesToDowngrade() const;
  unsigned packagesToReInstall() const;
  unsigned packagesToRemove() const;

  const ByteCount & toDownload() const		{ return _todownload; }
  const ByteCount & inCache() const		{ return _incache; }
  const ByteCount & installedSizeChange() const	{ return _inst_size_change; }

  /** The exposed needMachineReboot value causing ZYPPER_EXIT_INF_REBOOT_NEEDED considers patches only (zypper#237)
   * Packages cause a summary hint but will not lead to a non-zero return value.
   */
  bool needMachineReboot() const		{ return _need_reboot_patch; }
  /** The exposed needPkgMgrRestart value causing ZYPPER_EXIT_INF_RESTART_NEEDED considers patches only (zypper#237)
   */
  bool needPkgMgrRestart() const		{ return _need_restart; }


  void dumpTo( std::ostream & out );
  void dumpAsXmlTo( std::ostream & out );

private:
  void readPool( const ResPool & pool );

  bool writeResolvableList( std::ostream & out, const ResPairSet & resolvables, ansi::Color = ansi::Color::nocolor(), unsigned maxEntries_r = 0U, bool withKind_r = false );
  bool writeResolvableList( std::ostream & out, const ResPairSet & resolvables, unsigned maxEntries_r, bool withKind_r = false )
  { return writeResolvableList( out, resolvables, ansi::Color::nocolor(), maxEntries_r, withKind_r ); }

  void writeXmlResolvableList( std::ostream & out, const KindToResPairSet & resolvables );

  void collectInstalledRecommends( const ResObject::constPtr & obj );

private:
  ViewOptions _viewop;
  mutable unsigned _wrap_width;
  bool _force_no_color;
  bool _download_only;

  bool _need_reboot_patch;	// need_reboot caused by a patch
  bool _need_reboot_nonpatch;	// need_reboot caused by something not a patch
  bool _need_restart;		// by now patch specific

  ByteCount _todownload;
  ByteCount _incache;
  ByteCount _inst_size_change;

  // STATS

  unsigned _inst_pkg_total;

  KindToResPairSet _toinstall;
  KindToResPairSet _toupgrade;
  KindToResPairSet _todowngrade;
  KindToResPairSet _toreinstall;
  KindToResPairSet _toremove;
  KindToResPairSet _tochangearch;
  KindToResPairSet _tochangevendor;
  /** Packages that have update candidate, but won't get updated.
   * In 'zypper up' this is because of vendor, repo priority, dependiencies,
   * etc; but the list can be used also generally. */
  KindToResPairSet _notupdated;
  /** objects from previous lists with unknown support status */
  KindToResPairSet _supportUnknown;
  KindToResPairSet _supportUnsupported;	///< known to be unsupported
  KindToResPairSet _supportNeedACC;	///< need additional contract
  /** Patches and Packages which require a reboot */
  KindToResPairSet _rebootNeeded;

  /** names of packages which have multiple versions (to-be-)installed */
  std::set<std::string> _multiInstalled;

  std::list<std::string> _ctc;		///< reasons to consider to cancel


  /** \name For weak deps info.
   * @{
   */
  KindToResPairSet _required;
  KindToResPairSet _recommended;
  //! recommended but not to be installed
  KindToResPairSet _noinstrec;
  //! suggested but not to be installed
  KindToResPairSet _noinstsug;
  //! @}
};

#endif /* ZYPPER_UTILS_SUMMARY_H_ */
