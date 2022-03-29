/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_MEDIA_NG_WORKER_MOUNTINGWORKER_H_INCLUDED
#define ZYPP_MEDIA_NG_WORKER_MOUNTINGWORKER_H_INCLUDED

#include <zypp-media/ng/worker/ProvideWorker>
#include <zypp-core/zyppng/base/Signals>
#include <any>
#include <unordered_map>

namespace zyppng::worker
{

  struct Device
  {
    std::string  _name;           //!< Path of the device node or URL for e.g. nfs devices
    unsigned int _maj_nr    = 0;  //!< Major number of the device
    unsigned int _min_nr    = 0;  //!< Minor number of the device
    zypp::Pathname _mountPoint = {}; //!< Mountpoint of the device, if empty dev is not mounted
    bool _ephemeral = false;      //!< If set to true the device is removed from the internal list after the last attachpoint was released
    std::unordered_map<std::string, std::any> _properties = {};
  };

  struct AttachedMedia
  {
    std::shared_ptr<Device> _dev;
    zypp::Pathname _attachRoot;
  };

  class MountingWorker : public zyppng::worker::ProvideWorker
  {
    public:
      MountingWorker( zyppng::worker::WorkerCaps::WorkerType wType, std::string_view workerName );
      ~MountingWorker();

      void immediateShutdown() override;

    protected:

      /*!
       * Called to populate the device array when the first provide request arrives, only
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
       * Called by the provide loop whenever a attach request is received.
       */
      virtual void handleMountRequest ( ProvideWorkerItem &msg ) = 0;

      /*!
       * Forcefully unmounts the device, this does not check if there any attached medias still relying on it
       */
      virtual void unmountDevice ( Device &dev );

      /*!
       * Returns true if the worker handles volatile devices ( e.g. DVDs ).
       * The default impl returns false.
       */
      virtual bool isVolatile () const;

      /*!
       * Returns the \a attachRoot as dictated by the controller
       */
      const zypp::Pathname &attachRoot () const;

      /*!
       * Checks if the medium \a deviceUrl mounted on \a path matches the \a verifier and \a mediaNr
       */
      zyppng::expected<void> isDesiredMedium ( const zypp::Url &deviceUrl, const zypp::Pathname &mountPoint, const zyppng::MediaDataVerifierRef &verifier, uint mediaNr = 1 );

      // ProvideWorker interface
      zyppng::expected<zyppng::worker::WorkerCaps> initialize(const zyppng::worker::Configuration &conf) override;
      void provide() override;
      void cancel( const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator &i ) override;

    private:
      zyppng::worker::WorkerCaps::WorkerType _wtype;
      zypp::Pathname _attachRoot;
      std::vector<std::shared_ptr<Device>> _sysDevs;
      std::unordered_map<std::string, AttachedMedia> _attachedMedia;
      bool _devicesDetected = false; //< We delay device detection to the first attach request, to avoid doing it without needing it
  };
}

#endif
