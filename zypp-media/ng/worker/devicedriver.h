/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_MEDIA_NG_WORKER_DEVICEDRIVER_H_INCLUDED
#define ZYPP_MEDIA_NG_WORKER_DEVICEDRIVER_H_INCLUDED

#include <zypp-media/ng/ProvideFwd>
#include <zypp-media/ng/worker/ProvideWorker>
#include <zypp-media/ng/HeaderValueMap>
#include <zypp-media/Mount>
#include <zypp-core/zyppng/base/Signals>
#include <zypp-core/zyppng/base/Base>
#include <zypp-core/zyppng/base/zyppglobal.h>
#include <zypp-core/zyppng/pipelines/Expected>
#include <any>
#include <unordered_map>

namespace zyppng::worker
{

  ZYPP_FWD_DECL_TYPE_WITH_REFS (DeviceDriver);

  struct Device
  {
    std::string  _name;             //!< Path of the device node or URL for e.g. nfs devices
    unsigned int _maj_nr    = 0;    //!< Major number of the device
    unsigned int _min_nr    = 0;    //!< Minor number of the device
    zypp::Pathname _mountPoint = {};//!< Mountpoint of the device, if empty dev is not mounted
    bool _ephemeral = false;        //!< If set to true the device is removed from the internal list after the last attachpoint was released
    std::unordered_map<std::string, std::any> _properties = {};
  };

  struct AttachedMedia
  {
    std::shared_ptr<Device> _dev;
    zypp::Pathname _attachRoot;
  };

  struct AttachError
  {
    AttachError ( const uint code, const std::string &reason, const bool transient, const HeaderValueMap &extra = {} );
    AttachError ( const uint code, const bool transient, const zypp::Exception &e );

    uint _code;
    std::string _reason;
    bool _transient;
    HeaderValueMap _extra;
  };

  using AttachResult = expected<void, AttachError>;

  /*!
   * Abstract base class to be used together with the \sa MountingWorker class to control
   * attaching and detaching of a multitude of different media types via a unified interface.
   * Reimplement this class to easily support a new backend that utilizes the "mount" command to
   * attach real filesystems to the system.
   */
  class DeviceDriver : public zyppng::Base
  {
    public:

      DeviceDriver ( WorkerCaps::WorkerType wType );

      /*!
       * Tells the driver which provide worker to use when requiring auth data or media changes
       */
      void setProvider ( ProvideWorkerWeakRef workerRef );

      /*!
       * Called by the provide loop whenever a attach request is received.
       */
      virtual AttachResult mountDevice ( const uint32_t id, const zypp::Url &mediaUrl, const std::string &attachId, const std::string &label, const HeaderValueMap &extras ) = 0;



      virtual zyppng::expected<WorkerCaps> initialize(const zyppng::worker::Configuration &conf);


      /*!
       * Detaches the medium referenced by \a attachId.
       * Returns false if the medium could not be found
       *
       * \note this will not unmount the physical devices.
       *       Call \ref releaseIdleDevices to force this.
       *
       */
      bool detachMedia ( const std::string &attachId );

      /*!
       * Physically detaches all devices that are not referenced by a attachment anymore
       */
      void releaseIdleDevices ();

      /*!
       * Called by the parent to populate the device array every time a provide request arrives, only
       * relevant for workers that operate on detectable devices.
       * The base implementation does nothing
       */
      virtual void detectDevices();

      /*!
       * Returns a list of all currently known devices, a subclass should always make sure
       * that all currently mounted devices are present in that list.
       */
      std::vector<std::shared_ptr<Device>> &knownDevices();

      /*!
       * Returns a list of all currently known devices, a subclass should always make sure
       * that all currently mounted devices are present in that list.
       */
      const std::vector<std::shared_ptr<Device>> &knownDevices() const;

      /*!
       * Returns the list of currently attached medias
       */
      std::unordered_map<std::string, AttachedMedia> &attachedMedia();

      /*!
       * Returns true if the worker handles volatile devices ( e.g. DVDs ).
       * The default impl returns false.
       */
      virtual bool isVolatile () const;

      /*!
       * Changes the attach root to a specific path, otherwise realpath(".") is used.
       */
      void setAttachRoot ( const zypp::Pathname &root );

      /*!
       * Returns the \a attachRoot as dictated by the controller
       */
      zypp::Pathname attachRoot () const;

      /*!
       * The system is shutting down, release all ressources
       */
      virtual void immediateShutdown();

      /*!
       * Returns the parent worker if set
       */
      ProvideWorkerRef parentWorker () const;

      /*!
       * Returns the configuration that was sent by the controller
       */
      const zyppng::worker::Configuration &config() const;

    protected:
      /*!
       * Forcefully unmounts the device, this does not check if there any attached medias still relying on it
       */
      virtual void unmountDevice ( Device &dev );

      /*!
       * Checks if the medium \a deviceUrl mounted on \a path matches the \a verifier and \a mediaNr
       */
      zyppng::expected<void> isDesiredMedium ( const zypp::Url &deviceUrl, const zypp::Pathname &mountPoint, const zyppng::MediaDataVerifierRef &verifier, uint mediaNr = 1 );


      zypp::Pathname createAttachPoint(const zypp::Pathname &attach_root) const;
      void removeAttachPoint ( const zypp::Pathname &attach_pt ) const;
      bool checkAttached ( const zypp::Pathname &mountPoint, const std::function<bool( const zypp::media::MountEntry &)> predicate );

      /*!
      * Returns a predicate for the \ref checkAttached function that looks for a real device in the mount table
      */
      static const std::function<bool( const zypp::media::MountEntry &)> devicePredicate ( unsigned int majNr,  unsigned int minNr );

      /*!
      * Returns a predicate for the \ref checkAttached function that looks for a virtual mount ( like smb or nfs ) in the mount table
      */
      static const std::function<bool( const zypp::media::MountEntry &)> fstypePredicate ( const std::string &src, const std::vector<std::string> &fstypes );

      /*!
      * Returns a predicate for the \ref checkAttached function that looks for a bind mount in the mount table
      */
      static const std::function<bool( const zypp::media::MountEntry &)> bindMountPredicate ( const std::string &src );

    private:
      WorkerCaps::WorkerType _wType;
      zyppng::worker::Configuration _config;
      time_t  _attach_mtime = 0; //< Timestamp of the mtab we did read
      zypp::Pathname _attachRoot;
      std::vector<std::shared_ptr<Device>> _sysDevs;
      std::unordered_map<std::string, AttachedMedia> _attachedMedia;
      ProvideWorkerWeakRef _parentWorker;
  };




}

#endif
