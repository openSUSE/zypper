/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/hal/HalContext.h
 *
 *  \brief Hardware abstaction layer library wrapper.
 */
#ifndef ZYPP_TARGET_HAL_HALCONTEXT_H
#define ZYPP_TARGET_HAL_HALCONTEXT_H

#include "zypp/target/hal/HalException.h"
#include "zypp/base/PtrTypes.h"
#include <string>
#include <vector>
#include <stdint.h>

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace target
  { //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    namespace hal
    { ////////////////////////////////////////////////////////////////


      // -------------------------------------------------------------
      /** @{
       * Forward declarations.
       */
      class HalDrive;
      class HalVolume;

      class HalDrive_Impl;
      class HalVolume_Impl;
      class HalContext_Impl;
      /** }@ */


      ////////////////////////////////////////////////////////////////
      //
      // CLASS NAME : HalContext
      //
      /** Hardware abstaction layer context.
       *
       * Hal context wrapper. It manages the dbus connection and is
       * the entry point to query drives, volumes and other information.
       *
       * @todo: wrap more functions.
       */
      class HalContext
      {
      public:
        typedef
        zypp::RW_pointer<HalContext_Impl>::unspecified_bool_type  bool_type;

        HalContext(bool autoconnect=false);
        HalContext(const HalContext &context);
        ~HalContext();

        HalContext&
        operator=(const HalContext &context);

        /**
         * Verifies if the context is initialized.
         */
        operator bool_type() const;

        /**
         */
        void
        connect();

        /**
         * Retrieve UDI's of all devices.
         * \return Vector with device UDI's.
         */
        std::vector<std::string>
        getAllDevices() const;

        /**
         * Construct a HalDrive object for the specified UDI.
         * \param  The \p udi of the drive.
         * \return The HalDrive object.
         */
        HalDrive
        getDriveFromUDI(const std::string &udi) const;

        /**
         * Construct a HalVolume object for the specified UDI.
         * \param  The \p udi of the volume.
         * \return The HalVolume object.
         */
        HalVolume
        getVolumeFromUDI(const std::string &udi) const;

        HalVolume
        getVolumeFromDeviceFile(const std::string &device_file) const;

        /**
         * Retrieve UDI's of all devices with a capability.
         * \param  The \p capability name
         * \return Vector with device UDI's.
         */
        std::vector<std::string>
        findDevicesByCapability(const std::string &capability) const;

        bool
        getDevicePropertyBool  (const std::string &udi,
                                const std::string &key) const;

        int32_t
        getDevicePropertyInt32 (const std::string &udi,
                                const std::string &key) const;

        uint64_t
        getDevicePropertyUInt64(const std::string &udi,
                                const std::string &key) const;

        double
        getDevicePropertyDouble(const std::string &udi,
                                const std::string &key) const;

        std::string
        getDevicePropertyString(const std::string &udi,
                                const std::string &key) const;

        void
        setDevicePropertyBool  (const std::string &udi,
                                const std::string &key,
                                bool               value);

        void
        setDevicePropertyInt32 (const std::string &udi,
                                const std::string &key,
                                int32_t            value);

        void
        setDevicePropertyUInt64(const std::string &udi,
                                const std::string &key,
                                uint64_t           value);

        void
        setDevicePropertyDouble(const std::string &udi,
                                const std::string &key,
                                double             value);

        void
        setDevicePropertyString(const std::string &udi,
                                const std::string &key,
                                const std::string &value);

        void
        removeDeviceProperty(const std::string &udi,
                             const std::string &key);

      private:

        zypp::RW_pointer<HalContext_Impl> h_impl;
      };


      ////////////////////////////////////////////////////////////////
      //
      // CLASS NAME : HalDrive
      //
      /** Hardware abstaction layer storage drive object.
       *
       * @todo: wrap more functions.
       */
      class HalDrive
      {
      public:
        typedef
        zypp::RW_pointer<HalDrive_Impl>::unspecified_bool_type    bool_type;

        HalDrive();
        HalDrive(const HalDrive &drive);
        ~HalDrive();

        HalDrive&
        operator=(const HalDrive &drive);

        operator bool_type() const;

        std::string
        getUDI() const;

        std::string
        getTypeName() const;

        /**
         * \return The drive's device file name.
         */
        std::string
        getDeviceFile() const;

        /**
         * \return The drive's device file major number.
         */
        unsigned int
        getDeviceMajor() const;

        /**
         * \return The drive's device minor number.
         */
        unsigned int
        getDeviceMinor() const;

        /**
         * \return True, if drive uses removable media.
         */
        bool
        usesRemovableMedia() const;

        /*
        ** Returns the media type names supported by the drive.
        **
        ** Since hal does not implement a textual form here, we
        ** are using the drive type and property names from
        ** "storage.cdrom.*" namespace:
        **   cdrom, cdr, cdrw, dvd, dvdr, dvdrw, dvdram,
        **   dvdplusr, dvdplusrw, dvdplusrdl
        **
        ** FIXME: Should we provide own LibHalDriveCdromCaps?
        */
        std::vector<std::string>
        getCdromCapabilityNames() const;

        /**
         * Retrieve UDI's of all volumes of this drive.
         * \return Vector with volume UDI's.
         */
        std::vector<std::string>
        findAllVolumes() const;

      private:
        friend class HalContext;

        HalDrive(HalDrive_Impl *impl);

        zypp::RW_pointer<HalDrive_Impl>   d_impl;
      };


      ////////////////////////////////////////////////////////////////
      //
      // CLASS NAME : HalVolume
      //
      /** Hardware abstaction layer storage volume object.
       *
       * @todo: wrap more functions.
       */
      class HalVolume
      {
      public:
        typedef
        zypp::RW_pointer<HalVolume_Impl>::unspecified_bool_type   bool_type;

        HalVolume();
        HalVolume(const HalVolume &volume);
        ~HalVolume();

        HalVolume&
        operator=(const HalVolume &volume);

        operator bool_type() const;

        std::string
        getUDI() const;

        /**
         * \return The Volume drive's device file name.
         */
        std::string
        getDeviceFile() const;

        /**
         * \return The Volume drive's device major number.
         */
        unsigned int
        getDeviceMajor() const;

        /**
         * \return The Volume drive's device minor number.
         */
        unsigned int
        getDeviceMinor() const;

        bool
        isDisc() const;

        bool
        isPartition() const;

        bool
        isMounted() const;

        /**
         * \return The filesystem name on the volume.
         */
        std::string
        getFSType() const;

        /**
         * \return The filesystem usage purpose.
         */
        std::string
        getFSUsage() const;

        /**
         * \return The mount point of the volume.
         */
        std::string
        getMountPoint() const;

      private:
        friend class HalContext;
        friend class HalDrive;
        HalVolume(HalVolume_Impl *impl);

        zypp::RW_pointer<HalVolume_Impl>  v_impl;
      };


      ////////////////////////////////////////////////////////////////
    } // namespace hal
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
  } // namespace target
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif // ZYPP_TARGET_HAL_HALCONTEXT_H

/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
