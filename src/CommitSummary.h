/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_COMMITSUMMARY_H_
#define ZYPPER_UTILS_COMMITSUMMARY_H_

#include <zypp-core/base/NonCopyable.h>
#include <zypp/ZYppCommitResult.h>
#include <zypp/ResPool.h>
#include "utils/ansi.h"

class Zypper;

class CommitSummary : public zypp::base::NonCopyable
{

public:

  enum _view_options
  {
    DEFAULT                 = 0x0300,
    DETAILS                 = 0x00ff,

    SHOW_VERSION            = 0x0001,
    SHOW_ARCH               = 0x0002,
    SHOW_REPO               = 0x0004,
    SHOW_VENDOR             = 0x0008,

    SHOW_ALL                = 0xffff
  };

  typedef enum _view_options ViewOptions;


  CommitSummary( const zypp::ZYppCommitResult &result, const ViewOptions options = DEFAULT );
  ~CommitSummary() {}

  void setViewOptions( const ViewOptions options )	{ _viewop = options; }
  ViewOptions viewOptions() const			{ return _viewop; }
  void setViewOption( const ViewOptions option )	{ _viewop = (ViewOptions) (_viewop | option); }
  void unsetViewOption( const ViewOptions option )	{ _viewop = (ViewOptions) (_viewop & ~option); }
  void toggleViewOption( const ViewOptions option )	{ _viewop & option ? unsetViewOption(option) : setViewOption(option); }
  void setForceNoColor( bool value = true )		{ _force_no_color = value; }
  bool forceNoColor( ) const                  		{ return _force_no_color; }

  void dumpTo( std::ostream & out );
  void dumpAsXmlTo( std::ostream & out );

  static void showBasicErrorMessage ( Zypper &zypp );

protected:
  void collectData ();
  bool writeResolvableList( std::ostream &out, const std::vector<zypp::sat::Solvable> &resolvables, ansi::Color = ansi::Color::nocolor(), unsigned maxEntries_r = 0U );
  void writeFailedInstalls(std::ostream &out);
  void writeSkippedInstalls(std::ostream &out);
  void writeFailedRemovals(std::ostream &out);
  void writeSkippedRemovals(std::ostream &out);
  void writeXmlResolvableList(std::ostream &out, const std::vector<zypp::sat::Solvable> &solvables);
private:
  ViewOptions _viewop = DEFAULT;
  bool _force_no_color = false;
  mutable unsigned _wrap_width;
  const zypp::ZYppCommitResult &_result;

  bool _dataCollected = false;
  std::vector< zypp::sat::Solvable > _failedInstalls;
  std::vector< zypp::sat::Solvable > _failedRemovals;
  std::vector< zypp::sat::Solvable > _skippedInstalls;
  std::vector< zypp::sat::Solvable > _skippedRemovals;


};

#endif
