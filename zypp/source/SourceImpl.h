/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/SourceImpl.h
 *
*/
#ifndef ZYPP_SOURCE_SOURCEIMPL_H
#define ZYPP_SOURCE_SOURCEIMPL_H

#include <iosfwd>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/ProvideNumericId.h"

#include "zypp/Source.h"
#include "zypp/ResStore.h"
#include "zypp/Pathname.h"
#include "zypp/CheckSum.h"
#include "zypp/media/MediaManager.h"
#include "zypp/source/MediaSet.h"
#include "zypp/TmpPath.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(SourceImpl);
    
    /** Base class for Source prober.
     *
     * Source probe implementations get a attached media id
     * and return a bool, wether the source is a source
     * of this type
     */
    class SourceProber
    {
      public:
        SourceProber( media::MediaAccessId media_id, const Pathname &path )
          : _media_id(media_id), _path(path)
        {}
        
        virtual ~SourceProber()
        {}
        
        virtual bool operator()() = 0;

      protected:
        media::MediaAccessId _media_id;
        Pathname _path;
    };
    
    class SourceEventHandler
    {
      public:
        typedef boost::shared_ptr<SourceEventHandler> Ptr;
        SourceEventHandler( boost::function<void (int)> fnc )
        : _fnc(fnc)
        {
        
        };
        ~SourceEventHandler()
        {};
        void progress(int p)
        {
          if (_fnc)
            _fnc(p);
        }
      
      private:
        boost::function<void (int)> _fnc;
        int _file_size;
        int no_lines;
    };
    
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SourceImpl
    //
    /** Base class for concrete Source implementations.
     *
     * Public access via \ref Source_Ref interface.
     *
     * Constructed by \ref SourceFactory, via default ctor to
     * create the object, followed by a call to \ref factoryCtor.
     * \ref factoryCtor initializes the remaining data members and
     * calls \ref factoryInit to let implementations actually retrieve
     * the metadata.
     *
     * \todo Provide class NoSourceImpl and protect it against manipulation
     * of data via set methods (or erase them).
    */
    class SourceImpl : public base::ReferenceCounted
                     , public base::ProvideNumericId<SourceImpl,Source_Ref::NumericId>
                     , private base::NonCopyable
    {
      media::MediaManager media_mgr;

      friend std::ostream & operator<<( std::ostream & str, const SourceImpl & obj );

    public:
      /** Ctor substitute.
       * Called by SourceFactory to initialize the Source. Actions performed
       * are too complex for a real ctor. So factoryCtor initializes the
       * appropriate data members and then calls \ref factoryInit to
       * launch the Source.
       *
       * Common cleanup in case \ref factoryInit throws:
       * \li clear _store
       *
       * \throw EXCEPTION on fail
      */
      void factoryCtor( const media::MediaId & media_r,
                        const Pathname & path_r,
                        const std::string & alias_r,
                        const Pathname cache_dir_r,
			bool base_source, bool auto_refresh);

      /** SourceImpl MediaVerifier. */
      class Verifier;

    public:

      /** All resolvables provided by this source. */
      const ResStore & resolvables(Source_Ref) const
      { return resolvables(); }
      const ResStore & resolvables() const;
      const ResStore resolvables(zypp::Resolvable::Kind kind) const;

      /**
       * Provide a file to local filesystem
       *
       * \throws Exception
       *
       */
      const Pathname provideFile(const Pathname & file,
				 const unsigned media_nr = 1,
				 bool cached = false,
				 bool checkonly = false);
      /**
       * Provide a file to local filesystem
       * if the file does not exists, throws an exception, but
       * does not release the media, useful to provide
       * optional files you want to check if they exists
       *
       * \throws Exception
       *
       */
      const Pathname tryToProvideFile( const Pathname & file, const unsigned media_nr = 1 );

      /**
       * Provide a directory to local filesystem
       *
       * \throws Exception
       *
       */
      const Pathname provideDirTree(const Pathname & path,
                                const unsigned media_nr = 1);

      const void releaseFile(const Pathname & file_r,
			     const unsigned media_nr = 1);

      const void releaseDir(const Pathname & path_r,
			    const unsigned media_nr = 1,
			    const bool recursive = false);

      const Pathname providePackage( Package::constPtr package );

      /**
       * Provide info about a directory
       *
       * \throws Exception
       *
       */
      void dirInfo( const unsigned media_nr,
                    std::list<std::string> &retlist,
                    const Pathname         &path_r,
                    bool                    dots = true) const;

      void changeMedia(const media::MediaId & media_r, const Pathname & path_r);

      const bool enabled() const
      { return _enabled; }

      void enable();

      void disable()
      { _enabled = false; }

      const bool autorefresh() const
      { return _autorefresh; }

      void setAutorefresh( const bool enable_r )
      { _autorefresh = enable_r; }

      void refresh();

      virtual void storeMetadata(const Pathname & cache_dir_r);

      /**
        * default implementation returns now()
        * so the source is always reread when in doubt
        */
      virtual Date timestamp() const;

      virtual std::string checksum() const;
      
      std::string alias (void) const
      { return _alias; }

      void setAlias (const std::string & alias)
      { _alias = alias; }

      virtual std::string id (void) const;
      virtual void setId (const std::string id_r);
      virtual unsigned priority (void) const;
      virtual void setPriority (unsigned p);
      virtual unsigned priorityUnsubscribed (void) const;
      virtual void setPriorityUnsubscribed (unsigned p);
      virtual bool subscribed (void) const;
      virtual void setSubscribed (bool s);
      virtual const Pathname & cacheDir (void);
      virtual const std::list<Pathname> publicKeys();

      virtual std::string type(void) const;

      Url url (void) const;
      void setUrl( const Url & url );
      bool remote () const;
      bool baseSource () const
      { return _base_source; }

      const Pathname & path (void) const;

      virtual unsigned numberOfMedia(void) const;

      virtual std::string vendor (void) const;

      virtual std::string unique_id (void) const;

      /**
       * ZMD backend specific stuff
       * default source only provides dummy implementations
       */
      virtual std::string zmdName (void) const;
      virtual void setZmdName (const std::string name_r);
      virtual std::string zmdDescription (void) const;
      virtual void setZmdDescription (const std::string desc_r);

      virtual void redirect(unsigned media_nr, const Url & new_url);
      /**
       * Reattach the source if it is not mounted, but downloaded,
       * to different directory
       *
       * \throws Exception
       */
      void reattach(const Pathname &attach_point);
      /**
       * Release all medias attached by the source
       */
      void release();

      /**
       * Get media verifier for the specified medium. In the
       * default installation, an instance of media::NoVerifier is
       * returned. The specific implementations of the sources
       * should overload this method to return a proper verifier.
       *
       * \param media_nr number of the medium
       *
       * \return instance of a media verifier for the given medium.
       */
      virtual media::MediaVerifierRef verifier(unsigned media_nr);

      virtual std::set<zypp::Resolvable::Kind> resolvableKinds() const;
      
      /** Provide Source_Ref back to \c this. */
      Source_Ref selfSourceRef()
      { return Source_Ref( this ); }
    protected:


      /**
       * Provide a file to local filesystem on the given path,
       * no checking or progress information redirection.
       * used by \ref provideFile and \ref providePackage.
       * If \c checkonly is true, no media change callback
       * will be invoked.
       *
       * \param path file with a path to be provided by the source
       * \param media_nr number of the media to look for the path
       * \param cached provide a cached copy of the file, if available
       * \param checkonly just check if it is possible to provide the file
       *
       * \return the local path for the provided file
       *
       * \throws Exception
       *
       */
      const Pathname provideJustFile(const Pathname & path,
				 const unsigned media_nr = 1,
				 bool cached = false,
				 bool checkonly = false);

      void copyLocalMetadata(const Pathname &src, const Pathname &dst) const;

      /**
       * function that creates the tmp metadata dir if it was not created.
       * this directory is used when cache_dir is not set (design flaw FIXME)
       */
      Pathname tmpMetadataDir() const;

        /**
         * reset the media verifier to no verifier
         */
        void resetMediaVerifier();
        
        /**
         * checks if a file exists in cache
         * if no, downloads it, copies it in given destination, and check matching checksum
         * if yes, compares checksum and copies it to destination locally
         * \throw EXCEPTION on download/copy failure and user abort
         */
        void getPossiblyCachedMetadataFile( const Pathname &file_to_download, const Pathname &destination, const Pathname &cached_file, const CheckSum &checksum );

    protected:
      /** All resolvables provided by this source. */
      ResStore _store;
      /** URL of the media */
      Url _url;
      /** Path to the source on the media */
      Pathname _path;
      /** The source is enabled */
      bool _enabled;
      /** If the source metadata should be autorefreshed */
      bool _autorefresh;
      /** (user defined) alias of the source */
      std::string _alias;
      /** Directory holding metadata cache */
      Pathname _cache_dir;
      /** (user defined) id of the source
          mostly used for ZENworks */
      std::string _id;
      /** (user defined) default priority of the source */
      unsigned _priority;
      /** (user defined) unsubscribed priority of the source */
      unsigned _priority_unsubscribed;
      /** subscribed?, solver prefers subscribed sources */
      bool _subscribed;
      /** source contains base product? */
      bool _base_source;

    private:
      mutable shared_ptr<filesystem::TmpDir> _tmp_metadata_dir_ptr;
      ///////////////////////////////////////////////////////////////////
      // no playground below this line ;)
      ///////////////////////////////////////////////////////////////////
    protected:
      /** Default Ctor.
       * Just create the object and prepare the data members. Then wait
       * for the \ref factoryCtor call to launch the Source.
      */
      SourceImpl();

      /** Ctor substitute invoked by \ref factoryCtor.
       * Derived implementations use this to load the
       * metadata.
       *
       * Baseclass implementation could do tasks which are
       * common to all sources.
       *
       * \throw EXCEPTION on fail
      */
      virtual void factoryInit();

      /** Dtor. */
      virtual ~SourceImpl();

      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;

      /** Set of medias of the product */
      intrusive_ptr<MediaSet> _media_set;

    private:
      /** Late initialize the ResStore. */
      virtual void createResolvables(Source_Ref source_r);

      /** Provide only resolvable of a certain kind. */
      virtual ResStore provideResolvablesByKind(Source_Ref source_r, zypp::Resolvable::Kind kind);

      /** Whether the ResStore is initialized. */
      bool _res_store_initialized;

    public:
      /** Whether the ResStore is initialized.
       * If we know that noone has seen the resolvables yet, we can skip
       * them too, eg. when deleting a source. (#174840)
       */
      bool resStoreInitialized () const
      { return _res_store_initialized; }

    private:
      /** Helper indicating creation of nullimpl. */
      struct null {};

      /** Ctor, excl. for nullimpl only.
       * Nullimpl has no Id (\c 0).
      */
      SourceImpl( const null & );

    public:
      /** Offer default Impl. */
      static SourceImpl_Ptr nullimpl()
      {
        static SourceImpl_Ptr _nullimpl( new SourceImpl( null() ) );
        return _nullimpl;
      }

    };
    ///////////////////////////////////////////////////////////////////

    /** \relates SourceImpl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const SourceImpl & obj )
    { return obj.dumpOn( str ); }

    ///////////////////////////////////////////////////////////////////

    /** SourceImpl MediaVerifier.
    */
    class SourceImpl::Verifier : public media::MediaVerifierBase
      {
      public:
	/** ctor */
	Verifier (const std::string & vendor_r, const std::string & id_r, const media::MediaNr media_nr = 1);
	/*
	 ** Check if the specified attached media contains
	 ** the desired media number (e.g. SLES10 CD1).
	 */
	virtual bool
	isDesiredMedia(const media::MediaAccessRef &ref);

      private:
	std::string _media_vendor;
	std::string _media_id;
	media::MediaNr _media_nr;
	SourceImpl_Ptr _source;
      };

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SOURCEIMPL_H
