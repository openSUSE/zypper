/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_PACKAGE_CALLBACKS_H
#define ZMART_PACKAGE_CALLBACKS_H

#include <zypp/ZYppCallbacks.h>
#include <zypp/base/Logger.h>

#include "Zypper.h"

namespace ZmartRecipients
{
  struct PackageHeadReportReceiver : public zypp::callback::ReceiveReport<zypp::PackageHeadReport>
  {
    virtual void report ( const UserData &data_r )
    {
      if ( data_r.type() == zypp::ContentType( zypp::PackageHeadReport::ACCEPT_PACKAGE_HEAD_DOWNLOAD ) ) {
        if ( !data_r.hasvalue("PackageName") ) {
          WAR << "Missing arguments in report call for content type: " << data_r.type() << endl;
          return;
        }
        const std::string &name  = data_r.get<std::string>("PackageName");
        bool res = askUserAboutPackageHeadDownload( name );
        data_r.set("Response", res);
        return;
      }
      WAR << "Unhandled report() call" << endl;
    }

    bool askUserAboutPackageHeadDownload( const std::string &packageName_r ) {

      Zypper::instance().out().info(
        str::form( _("Some information is not available for package %s."), packageName_r.c_str() )
      );

      return read_bool_answer(
        PROMPT_YN_DOWNLOAD_RPM_HEADER, _("Do you want to download the RPM header?"), true);
    }
  };
}

class PackageCallbacks {

  private:
    ZmartRecipients::PackageHeadReportReceiver _packageHeadDownloadReport;

  public:
    PackageCallbacks()
    {
      MIL << "Set package callbacks.." << std::endl;
      _packageHeadDownloadReport.connect();
    }

    ~PackageCallbacks()
    {
      _packageHeadDownloadReport.disconnect();
    }
};

#endif
