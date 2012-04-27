/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/hal/HalContext.cc
 *
 *  \brief Hardware abstaction layer library wrapper implementation.
 */
#include "zypp/target/hal/HalException.h"
//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace target
  { //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    namespace hal
    { ////////////////////////////////////////////////////////////////
      NoHalException::NoHalException()
        : Exception(_("Sorry, but this version of libzypp was built without HAL support."))
      {}
      ////////////////////////////////////////////////////////////////
    } // namespace hal
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
  } // namespace target
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#ifndef NO_HAL // disables zypp's HAL dependency

#include "zypp/target/hal/HalContext.h"
#include "zypp/thread/Mutex.h"
#include "zypp/thread/MutexLock.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include <hal/libhal.h>
#include <hal/libhal-storage.h>

#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace target
  { //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    namespace hal
    { ////////////////////////////////////////////////////////////////

      using zypp::thread::Mutex;
      using zypp::thread::MutexLock;

      ////////////////////////////////////////////////////////////////
      namespace // anonymous
      { //////////////////////////////////////////////////////////////


        //////////////////////////////////////////////////////////////
        // STATIC
        /**
         ** hmm... currently a global one..
        */
        static Mutex g_Mutex;


        //////////////////////////////////////////////////////////////
        /**
         * Internal hal (dbus ) error helper class.
         */
        class HalError
        {
        public:
          DBusError error;

          HalError()  { dbus_error_init(&error); }
          ~HalError() { dbus_error_free(&error); }

          inline bool         isSet() const
          {
            return dbus_error_is_set(&error);
          }

          inline HalException halException(const std::string &msg = std::string()) const
          {
            if( isSet() && error.name != NULL && error.message != NULL) {
              return HalException(error.name, error.message);
            }
            else if( !msg.empty()) {
              return HalException(msg);
            }
            else {
              return HalException();
            }
          }
        };


        // -----------------------------------------------------------
        inline void
        VERIFY_CONTEXT(const zypp::RW_pointer<HalContext_Impl> &h)
        {
          if( !h)
          {
            ZYPP_THROW(HalException(_("HalContext not connected")));
          }
        }

        // -----------------------------------------------------------
        inline void
        VERIFY_DRIVE(const zypp::RW_pointer<HalDrive_Impl> &d)
        {
          if( !d)
          {
            ZYPP_THROW(HalException(_("HalDrive not initialized")));
          }
        }

        // -----------------------------------------------------------
        inline void
        VERIFY_VOLUME(const zypp::RW_pointer<HalVolume_Impl> &v)
        {
          if( !v)
          {
            ZYPP_THROW(HalException(_("HalVolume not initialized")));
          }
        }

        //////////////////////////////////////////////////////////////
      } // anonymous
      ////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////
      std::ostream &
      HalException::dumpOn( std::ostream & str ) const
      {
        if(!e_name.empty() && !e_msg.empty())
          return str << msg() << ": " << e_msg << " (" << e_name << ")";
        else if(!e_msg.empty())
          return str << msg() << ": " << e_msg;
        else
          return str << msg();
      }

      ////////////////////////////////////////////////////////////////
      class HalContext_Impl
      {
      public:
        HalContext_Impl();
        ~HalContext_Impl();

        DBusConnection *conn;
        LibHalContext  *hctx;
        bool            pcon; // private connection
      };


      ////////////////////////////////////////////////////////////////
      class HalDrive_Impl
      {
      public:
        zypp::RW_pointer<HalContext_Impl>  hal;
        LibHalDrive                       *drv;

        HalDrive_Impl()
          : hal(), drv(NULL)
        {
        }

        HalDrive_Impl(const zypp::RW_pointer<HalContext_Impl> &r,
                      LibHalDrive *d)
          : hal(r), drv(d)
        {
        }

        ~HalDrive_Impl()
        {
          if( drv)
            libhal_drive_free(drv);
        }
      };


      ////////////////////////////////////////////////////////////////
      class HalVolume_Impl
      {
      public:
        LibHalVolume *vol;

        HalVolume_Impl(LibHalVolume *v=NULL)
          : vol(v)
        {
        }

        ~HalVolume_Impl()
        {
          if( vol)
            libhal_volume_free(vol);
        }
      };


      ////////////////////////////////////////////////////////////////
      HalContext_Impl::HalContext_Impl()
        : conn(NULL)
        , hctx(NULL)
        , pcon(false) // we allways use shared connections at the moment
      {
        HalError err;

        if( pcon)
          conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err.error);
        else
          conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err.error);
        if( !conn) {
          ZYPP_THROW(err.halException(
            _("Unable to create dbus connection")
          ));
        }

        hctx = libhal_ctx_new();
        if( !hctx)
        {
          if( pcon)
              dbus_connection_close(conn);
          dbus_connection_unref(conn);
          conn = NULL;

          ZYPP_THROW(HalException(
            _("libhal_ctx_new: Can't create libhal context")
          ));
        }

        if( !libhal_ctx_set_dbus_connection(hctx, conn))
        {
          libhal_ctx_free(hctx);
          hctx = NULL;

          if( pcon)
            dbus_connection_close(conn);
          dbus_connection_unref(conn);
          conn = NULL;

          ZYPP_THROW(HalException(
            _("libhal_set_dbus_connection: Can't set dbus connection")
          ));
        }

        if( !libhal_ctx_init(hctx, &err.error))
        {
          libhal_ctx_free(hctx);
          hctx = NULL;

          if( pcon)
            dbus_connection_close(conn);
          dbus_connection_unref(conn);
          conn = NULL;

          ZYPP_THROW(err.halException(
            _("Unable to initalize HAL context -- hald not running?")
          ));
        }
      }

      // -------------------------------------------------------------
      HalContext_Impl::~HalContext_Impl()
      {
        if( hctx)
        {
          HalError err;
          libhal_ctx_shutdown(hctx, &err.error);
          libhal_ctx_free( hctx);
        }
        if( conn)
        {
          if( pcon)
            dbus_connection_close(conn);
          dbus_connection_unref(conn);
        }
      }


      ////////////////////////////////////////////////////////////////
      HalContext::HalContext(bool autoconnect)
        : h_impl( NULL)
      {
        MutexLock lock(g_Mutex);

        if( autoconnect)
          h_impl.reset( new HalContext_Impl());
      }

      // -------------------------------------------------------------
      HalContext::HalContext(const HalContext &context)
        : h_impl( NULL)
      {
        MutexLock lock(g_Mutex);

        zypp::RW_pointer<HalContext_Impl>(context.h_impl).swap(h_impl);
      }

      // -------------------------------------------------------------
      HalContext::~HalContext()
      {
        MutexLock  lock(g_Mutex);

        h_impl.reset();
      }

      // --------------------------------------------------------------
      HalContext &
      HalContext::operator=(const HalContext &context)
      {
        MutexLock  lock(g_Mutex);

        if( this == &context)
          return *this;

        zypp::RW_pointer<HalContext_Impl>(context.h_impl).swap(h_impl);
        return *this;
      }

      // --------------------------------------------------------------
      HalContext::operator HalContext::bool_type() const
      {
        MutexLock  lock(g_Mutex);

        return h_impl;
      }

      // --------------------------------------------------------------
      void
      HalContext::connect()
      {
        MutexLock lock(g_Mutex);

        if( !h_impl)
          h_impl.reset( new HalContext_Impl());
      }

      // --------------------------------------------------------------
      std::vector<std::string>
      HalContext::getAllDevices() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError   err;
        char     **names;
        int        count = 0;

        names = libhal_get_all_devices( h_impl->hctx, &count, &err.error);
        if( !names)
        {
          ZYPP_THROW(err.halException());
        }

        std::vector<std::string> ret(names, names + count);
        libhal_free_string_array(names);
        return ret;
      }

      // --------------------------------------------------------------
      HalDrive
      HalContext::getDriveFromUDI(const std::string &udi) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        LibHalDrive *drv = libhal_drive_from_udi(h_impl->hctx, udi.c_str());
        if( drv != NULL)
          return HalDrive(new HalDrive_Impl( h_impl, drv));
        else
          return HalDrive();
      }

      // --------------------------------------------------------------
      HalVolume
      HalContext::getVolumeFromUDI(const std::string &udi) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        LibHalVolume *vol = libhal_volume_from_udi(h_impl->hctx, udi.c_str());
        if( vol)
          return HalVolume( new HalVolume_Impl(vol));
        else
          return HalVolume();
      }

      // --------------------------------------------------------------
      HalVolume
      HalContext::getVolumeFromDeviceFile(const std::string &device_file) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        LibHalVolume *vol = libhal_volume_from_device_file(h_impl->hctx,
                                                           device_file.c_str());
        if( vol)
          return HalVolume( new HalVolume_Impl(vol));
        else
          return HalVolume();
      }

      // --------------------------------------------------------------
      std::vector<std::string>
      HalContext::findDevicesByCapability(const std::string &capability) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError   err;
        char     **names;
        int        count = 0;

        names = libhal_find_device_by_capability(h_impl->hctx,
                                                 capability.c_str(),
                                                 &count, &err.error);
        if( !names)
        {
          ZYPP_THROW(err.halException());
        }

        std::vector<std::string> ret(names, names + count);
        libhal_free_string_array(names);
        return ret;
      }

      // --------------------------------------------------------------
      bool
      HalContext::getDevicePropertyBool  (const std::string &udi,
                                          const std::string &key) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        dbus_bool_t   ret;

        ret = libhal_device_get_property_bool  (h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                &err.error);
        if( err.isSet())
        {
          ZYPP_THROW(err.halException());
        }
        return ret;
      }

      // --------------------------------------------------------------
      int32_t
      HalContext::getDevicePropertyInt32 (const std::string &udi,
                                          const std::string &key) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        dbus_int32_t  ret;

        ret = libhal_device_get_property_int   (h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                &err.error);
        if( err.isSet())
        {
          ZYPP_THROW(err.halException());
        }
        return ret;
      }

      // --------------------------------------------------------------
      uint64_t
      HalContext::getDevicePropertyUInt64(const std::string &udi,
                                          const std::string &key) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        dbus_uint64_t ret;

        ret = libhal_device_get_property_uint64(h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                &err.error);
        if( err.isSet())
        {
          ZYPP_THROW(err.halException());
        }
        return ret;
      }

      // --------------------------------------------------------------
      double
      HalContext::getDevicePropertyDouble(const std::string &udi,
                                          const std::string &key) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        double        ret;

        ret = libhal_device_get_property_bool  (h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                &err.error);
        if( err.isSet())
        {
          ZYPP_THROW(err.halException());
        }
        return ret;
      }


      // --------------------------------------------------------------
      std::string
      HalContext::getDevicePropertyString(const std::string &udi,
                                          const std::string &key) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        std::string   ret;
        char         *ptr;

        ptr = libhal_device_get_property_string(h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                &err.error);
        if( err.isSet())
        {
          ZYPP_THROW(err.halException());
        }
        if( ptr != NULL)
        {
          ret = ptr;
          free(ptr);
        }
        return ret;
      }

      // --------------------------------------------------------------
      void
      HalContext::setDevicePropertyBool  (const std::string &udi,
                                          const std::string &key,
                                          bool               value)
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        dbus_bool_t   ret;

        ret = libhal_device_set_property_bool  (h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                value ? 1 : 0,
                                                &err.error);
        if( !ret)
        {
          ZYPP_THROW(err.halException());
        }
      }

      // --------------------------------------------------------------
      void
      HalContext::setDevicePropertyInt32 (const std::string &udi,
                                          const std::string &key,
                                          int32_t            value)
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        dbus_bool_t   ret;

        ret = libhal_device_set_property_int   (h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                value,
                                                &err.error);
        if( !ret)
        {
          ZYPP_THROW(err.halException());
        }
      }

      // --------------------------------------------------------------
      void
      HalContext::setDevicePropertyUInt64(const std::string &udi,
                                          const std::string &key,
                                          uint64_t           value)
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        dbus_bool_t   ret;

        ret = libhal_device_set_property_uint64(h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                value,
                                                &err.error);
        if( !ret)
        {
          ZYPP_THROW(err.halException());
        }
      }

      // --------------------------------------------------------------
      void
      HalContext::setDevicePropertyDouble(const std::string &udi,
                                          const std::string &key,
                                          double             value)
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        dbus_bool_t   ret;

        ret = libhal_device_set_property_double(h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                value,
                                                &err.error);
        if( !ret)
        {
          ZYPP_THROW(err.halException());
        }
      }

      // --------------------------------------------------------------
      void
      HalContext::setDevicePropertyString(const std::string &udi,
                                          const std::string &key,
                                          const std::string &value)
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        dbus_bool_t   ret;

        ret = libhal_device_set_property_string(h_impl->hctx,
                                                udi.c_str(),
                                                key.c_str(),
                                                value.c_str(),
                                                &err.error);
        if( !ret)
        {
          ZYPP_THROW(err.halException());
        }
      }

      // --------------------------------------------------------------
      void
      HalContext::removeDeviceProperty(const std::string &udi,
                                       const std::string &key)
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError      err;
        dbus_bool_t   ret;

        ret = libhal_device_remove_property(h_impl->hctx,
                                            udi.c_str(),
                                            key.c_str(),
                                            &err.error);
        if( !ret)
        {
          ZYPP_THROW(err.halException());
        }
      }

      ////////////////////////////////////////////////////////////////
      HalDrive::HalDrive()
        : d_impl( NULL)
      {
      }

      // --------------------------------------------------------------
      HalDrive::HalDrive(HalDrive_Impl *impl)
        : d_impl( NULL)
      {
        MutexLock  lock(g_Mutex);

        d_impl.reset(impl);
      }

      // --------------------------------------------------------------
      HalDrive::HalDrive(const HalDrive &drive)
        : d_impl( NULL)
      {
        MutexLock  lock(g_Mutex);

        zypp::RW_pointer<HalDrive_Impl>(drive.d_impl).swap(d_impl);
      }

      // --------------------------------------------------------------
      HalDrive::~HalDrive()
      {
        MutexLock  lock(g_Mutex);

        d_impl.reset();
      }

      // --------------------------------------------------------------
      HalDrive &
      HalDrive::operator=(const HalDrive &drive)
      {
        MutexLock  lock(g_Mutex);

        if( this == &drive)
          return *this;

        zypp::RW_pointer<HalDrive_Impl>(drive.d_impl).swap(d_impl);
        return *this;
      }

      // --------------------------------------------------------------
      HalDrive::operator HalDrive::bool_type() const
      {
        MutexLock  lock(g_Mutex);

        return d_impl;
      }

      // --------------------------------------------------------------
      std::string
      HalDrive::getUDI() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        const char *ptr = libhal_drive_get_udi(d_impl->drv);
        return std::string(ptr ? ptr : "");
      }

      // --------------------------------------------------------------
      std::string
      HalDrive::getTypeName() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        const char *ptr = libhal_drive_get_type_textual(d_impl->drv);
        return std::string(ptr ? ptr : "");
      }

      // --------------------------------------------------------------
      std::string
      HalDrive::getDeviceFile() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        return std::string(libhal_drive_get_device_file(d_impl->drv));
      }

      // --------------------------------------------------------------
      unsigned int
      HalDrive::getDeviceMajor() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        return libhal_drive_get_device_major(d_impl->drv);
      }

      // --------------------------------------------------------------
      unsigned int
      HalDrive::getDeviceMinor() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        return libhal_drive_get_device_minor(d_impl->drv);
      }

      // --------------------------------------------------------------
      bool
      HalDrive::usesRemovableMedia() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        return libhal_drive_uses_removable_media(d_impl->drv);
      }

      // --------------------------------------------------------------
      std::vector<std::string>
      HalDrive::getCdromCapabilityNames() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        std::vector<std::string> ret;
        LibHalDriveCdromCaps     caps;

        /*
        ** FIXME: there is no textual variant :-(
        **        using property key names...
        */
        caps = libhal_drive_get_cdrom_caps(d_impl->drv);

        if(caps & LIBHAL_DRIVE_CDROM_CAPS_CDROM)
          ret.push_back("cdrom");
        if(caps & LIBHAL_DRIVE_CDROM_CAPS_CDR)
          ret.push_back("cdr");
        if(caps & LIBHAL_DRIVE_CDROM_CAPS_CDRW)
          ret.push_back("cdrw");
        if(caps & LIBHAL_DRIVE_CDROM_CAPS_DVDRAM)
          ret.push_back("dvdram");
        if(caps & LIBHAL_DRIVE_CDROM_CAPS_DVDROM)
          ret.push_back("dvd");
        if(caps & LIBHAL_DRIVE_CDROM_CAPS_DVDR)
          ret.push_back("dvdr");
        if(caps & LIBHAL_DRIVE_CDROM_CAPS_DVDRW)
          ret.push_back("dvdrw");
        if(caps & LIBHAL_DRIVE_CDROM_CAPS_DVDPLUSR)
          ret.push_back("dvdplusr");
        if(caps & LIBHAL_DRIVE_CDROM_CAPS_DVDPLUSRW)
          ret.push_back("dvdplusrw");
        if(caps & LIBHAL_DRIVE_CDROM_CAPS_DVDPLUSRDL)
          ret.push_back("dvdplusrdl");

        return ret;

#if 0
        if( libhal_drive_get_type(d_impl->drv) != LIBHAL_DRIVE_TYPE_CDROM)
          ZYPP_THROW(HalException(_("Not a CDROM drive")));

        /*
        ** FIXME: we use property keys matching
        **          "storage.cdrom.cd*"
        **          "storage.cdrom.dvd*"
        ** but this may print other bool keys,
        ** that are not CDROM caps.
        */
        LibHalPropertySet         *props;
        HalError                   err;

        props = libhal_device_get_all_properties(d_impl->hal->hctx,
                                                 getUDI().c_str(),
                                                 &err.error);
        if( !props)
          ZYPP_THROW(err.halException());

        std::vector<std::string>   ret(1, getTypeName());
        std::string                key;
        std::string                dvd("storage.cdrom.dvd");
        std::string                cd ("storage.cdrom.cd");

        LibHalPropertySetIterator  it;
        for(libhal_psi_init(&it, props);
            libhal_psi_has_more(&it);
            libhal_psi_next(&it))
        {
          if( libhal_psi_get_type(&it) == LIBHAL_PROPERTY_TYPE_BOOLEAN &&
              libhal_psi_get_bool(&it))
          {
            key = libhal_psi_get_key(&it);
            if( key.compare(0, cd.size(), cd) == 0)
            {
              ret.push_back(key.substr(sizeof("storage.cdrom.")-1));
            }
            else
            if( key.compare(0, dvd.size(), dvd) == 0)
            {
              ret.push_back(key.substr(sizeof("storage.cdrom.")-1));
            }
          }
        }
        libhal_free_property_set(props);

        return ret;
#endif
      }

      // --------------------------------------------------------------
      std::vector<std::string>
      HalDrive::findAllVolumes() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        char     **names;
        int        count = 0;

        names = libhal_drive_find_all_volumes(d_impl->hal->hctx,
                                              d_impl->drv,
                                              &count);

        std::vector<std::string> ret;
        ret.assign(names, names + count);
        libhal_free_string_array(names);
        return ret;
      }


      ////////////////////////////////////////////////////////////////
      HalVolume::HalVolume()
        : v_impl( NULL)
      {}

      HalVolume::HalVolume(HalVolume_Impl *impl)
        : v_impl( NULL)
      {
        MutexLock  lock(g_Mutex);

        v_impl.reset(impl);
      }

      // --------------------------------------------------------------
      HalVolume::HalVolume(const HalVolume &volume)
        : v_impl( NULL)
      {
        MutexLock  lock(g_Mutex);

        zypp::RW_pointer<HalVolume_Impl>(volume.v_impl).swap(v_impl);
      }

      // --------------------------------------------------------------
      HalVolume::~HalVolume()
      {
        MutexLock  lock(g_Mutex);

        v_impl.reset();
      }

      // --------------------------------------------------------------
      HalVolume &
      HalVolume::operator=(const HalVolume &volume)
      {
        MutexLock  lock(g_Mutex);

        if( this == &volume)
          return *this;

        zypp::RW_pointer<HalVolume_Impl>(volume.v_impl).swap(v_impl);
        return *this;
      }

      // --------------------------------------------------------------
      HalVolume::operator HalVolume::bool_type() const
      {
        MutexLock  lock(g_Mutex);

        return v_impl;
      }

      // --------------------------------------------------------------
      std::string
      HalVolume::getUDI() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        const char *ptr = libhal_volume_get_udi(v_impl->vol);
        return std::string(ptr ? ptr : "");
      }

      // --------------------------------------------------------------
      std::string
      HalVolume::getDeviceFile() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return std::string(libhal_volume_get_device_file(v_impl->vol));
      }

      // --------------------------------------------------------------
      unsigned int
      HalVolume::getDeviceMajor() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return libhal_volume_get_device_major(v_impl->vol);
      }

      // --------------------------------------------------------------
      unsigned int
      HalVolume::getDeviceMinor() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return libhal_volume_get_device_minor(v_impl->vol);
      }

      // --------------------------------------------------------------
      bool
      HalVolume::isDisc() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return libhal_volume_is_disc(v_impl->vol);
      }

      // --------------------------------------------------------------
      bool
      HalVolume::isPartition() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return libhal_volume_is_partition(v_impl->vol);
      }

      // --------------------------------------------------------------
      bool
      HalVolume::isMounted() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return libhal_volume_is_mounted(v_impl->vol);
      }

      // --------------------------------------------------------------
      std::string
      HalVolume::getFSType() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return std::string( libhal_volume_get_fstype(v_impl->vol));
      }

      // --------------------------------------------------------------
      std::string
      HalVolume::getFSUsage() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        LibHalVolumeUsage usage( libhal_volume_get_fsusage(v_impl->vol));
        std::string       ret;
        switch( usage)
        {
          case  LIBHAL_VOLUME_USAGE_MOUNTABLE_FILESYSTEM:
            ret = "filesystem";
          break;
          case LIBHAL_VOLUME_USAGE_PARTITION_TABLE:
            ret = "partitiontable";
          break;
          case LIBHAL_VOLUME_USAGE_RAID_MEMBER:
            return "raid";
          break;
          case LIBHAL_VOLUME_USAGE_CRYPTO:
            ret = "crypto";
          break;
          case LIBHAL_VOLUME_USAGE_UNKNOWN:
          default:
          break;
        }
        return ret;
      }

      // --------------------------------------------------------------
      std::string
      HalVolume::getMountPoint() const
      {
        VERIFY_VOLUME(v_impl);

        return std::string( libhal_volume_get_mount_point(v_impl->vol));
      }


      ////////////////////////////////////////////////////////////////
    } // namespace hal
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
  } // namespace target
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
#else // NO_HAL
#include "zypp/target/hal/HalContext.h"
#include "zypp/target/hal/HalException.h"
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace target
  { //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    namespace hal
    { ////////////////////////////////////////////////////////////////

      std::ostream &
      HalException::dumpOn( std::ostream & str ) const
      { return str; }

      // --------------------------------------------------------------
      class HalContext_Impl
      {};
      class HalDrive_Impl
      {};
      class HalVolume_Impl
      {};

      // --------------------------------------------------------------
      HalContext::HalContext(bool)
      { ZYPP_THROW( NoHalException() ); }
      HalContext::~HalContext()
      {}
      HalContext &
      HalContext::operator=(const HalContext &)
      { return *this; }
      HalContext::operator HalContext::bool_type() const
      { return 0; }
      void
      HalContext::connect()
      {}
      std::vector<std::string>
      HalContext::getAllDevices() const
      { return std::vector<std::string>(); }
      HalDrive
      HalContext::getDriveFromUDI(const std::string &) const
      { return HalDrive(); }
      HalVolume
      HalContext::getVolumeFromUDI(const std::string &) const
      { return HalVolume(); }
      HalVolume
      HalContext::getVolumeFromDeviceFile(const std::string &) const
      { return HalVolume(); }
      std::vector<std::string>
      HalContext::findDevicesByCapability(const std::string &) const
      { return std::vector<std::string>(); }
      bool
      HalContext::getDevicePropertyBool(const std::string &, const std::string &) const
      { return false; }
      void
      HalContext::setDevicePropertyBool  (const std::string &, const std::string &, bool value)
      {}
      void
      HalContext::removeDeviceProperty(const std::string &, const std::string &)
      {}
      std::string
      HalContext::getDevicePropertyString(const std::string &, const std::string &) const
      { return ""; }
      // --------------------------------------------------------------
      HalDrive::HalDrive()
      { ZYPP_THROW( NoHalException() ); }
      HalDrive::~HalDrive()
      {}
      HalDrive &
      HalDrive::operator=(const HalDrive &)
      { return *this; }
      HalDrive::operator HalDrive::bool_type() const
      { return 0; }
      std::string
      HalDrive::getUDI() const
      { return std::string(); }
      std::string
      HalDrive::getTypeName() const
      { return std::string(); }
      std::string
      HalDrive::getDeviceFile() const
      { return std::string(); }
      unsigned int
      HalDrive::getDeviceMinor() const
      { return 0; }
      unsigned int
      HalDrive::getDeviceMajor() const
      { return 0; }
      bool
      HalDrive::usesRemovableMedia() const
      { return false; }
      std::vector<std::string>
      HalDrive::getCdromCapabilityNames() const
      { return std::vector<std::string>(); }
      std::vector<std::string>
      HalDrive::findAllVolumes() const
      { return std::vector<std::string>(); }

      // --------------------------------------------------------------
      HalVolume::HalVolume()
      { ZYPP_THROW( NoHalException() ); }
      HalVolume::~HalVolume()
      {}
      HalVolume &
      HalVolume::operator=(const HalVolume &)
      { return *this; }
      HalVolume::operator HalVolume::bool_type() const
      { return 0; }
      std::string
      HalVolume::getUDI() const
      { return std::string(); }
      std::string
      HalVolume::getDeviceFile() const
      { return std::string(); }
      unsigned int
      HalVolume::getDeviceMinor() const
      { return 0; }
      unsigned int
      HalVolume::getDeviceMajor() const
      { return 0; }
      bool
      HalVolume::isDisc() const
      { return false; }
      bool
      HalVolume::isPartition() const
      { return false; }
      bool
      HalVolume::isMounted() const
      { return false; }
      std::string
      HalVolume::getFSType() const
      { return std::string(); }
      std::string
      HalVolume::getFSUsage() const
      { return std::string(); }
      std::string
      HalVolume::getMountPoint() const
      { return std::string(); }

      ////////////////////////////////////////////////////////////////
    } // namespace hal
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
  } // namespace target
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
#endif // NO_HAL

/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
