/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/tagret/rpm/RpmCallbacks.h
 *
*/

#ifndef ZYPP_MEDIA_MEDIACALLBACKS_H
#define ZYPP_MEDIA_MEDIACALLBACKS_H

#include <iosfwd>

#include "zypp/Url.h"
#include "zypp/Callback.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

namespace zypp {
  namespace target {
    namespace rpm {

      ///////////////////////////////////////////////////////////////////
      // Reporting progress of package removing
      ///////////////////////////////////////////////////////////////////
      class RemovePkgReport : public HACK::Callback {
      public:
        virtual ~RemovePkgReport()
        {}
        /** Start the operation */
        virtual void start( const std::string & name )
        { }
        /**
         * Inform about progress
         * Return true on abort
         */
        virtual bool progress( unsigned percent )
        { return false; }
        /** Finish operation in case of success */
        virtual void end()
        { }
        /** Finish operatino in case of fail, report fail exception */
        virtual void end( Exception & excpt_r )
        { }
      };

      extern RemovePkgReport removePkgReport;
  
      ///////////////////////////////////////////////////////////////////
      // Reporting progress of package installation
      ///////////////////////////////////////////////////////////////////
      class RpmInstallReport : public HACK::Callback {
      public:
        virtual ~RpmInstallReport()
        {}
        /** Start the operation */
        virtual void start( const Pathname & name ) 
        { }
        /**
         * Inform about progress
         * Return true on abort
         */
        virtual bool progress( unsigned percent )
        { return false; }
        /** Finish operation in case of success */
        virtual void end()
        { }
        /** Finish operatino in case of fail, report fail exception */
        virtual void end( Exception & excpt_r )
        { }
      };

      extern RpmInstallReport rpmInstallReport;

      ///////////////////////////////////////////////////////////////////
      // Reporting database scanning
      ///////////////////////////////////////////////////////////////////
      class ScanDbReport : public HACK::Callback {
      public:
        virtual ~ScanDbReport()
        {}
        /** Start the operation */
        virtual void start() 
        { }
        /**
         * Inform about progress
         * Return true on abort
         */
        virtual bool progress( unsigned percent )
        { return false; }
        /** Finish operation in case of success */
        virtual void end()
        { }
        /** Finish operatino in case of fail, report fail exception */
        virtual void end( Exception & excpt_r )
        { }
      };
  
      extern ScanDbReport scanDbReport;

      ///////////////////////////////////////////////////////////////////
      // Reporting progress of database rebuild
      ///////////////////////////////////////////////////////////////////
      class RebuildDbReport : public HACK::Callback {
      public:
        virtual ~RebuildDbReport()
        {}
        /** Start the operation */
        virtual void start() 
        { }
        /**
         * Inform about progress
         * Return true on abort
         */
        virtual bool progress( unsigned percent )
        { return false; }
        /** Finish operation in case of success */
        virtual void end()
        { }
        /** Finish operatino in case of fail, report fail exception */
        virtual void end( Exception & excpt_r )
        { }
      };
  
      extern RebuildDbReport rebuildDbReport;

    } // namespace rpm
  } // namespace target
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIACALLBACKS_H
