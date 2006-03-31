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
#include "zypp/media/MediaManager.h"
#include "zypp/source/MediaSet.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(SourceImpl);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SourceImpl
    //
    /** Base class for concrete Source implementations.
     *
     * Public access via \ref Source interface.
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
                        const Pathname & path_r = "/",
                        const std::string & alias_r = "",
                        const Pathname cache_dir_r = "");

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
      bool remote () const;

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
       * Get media verifier for the specified media
       */
      virtual media::MediaVerifierRef verifier(unsigned media_nr);

    protected:
      /** Provide Source_Ref back to \c this. */
      Source_Ref selfSourceRef()
      { return Source_Ref( this ); }

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
      virtual ResStore provideResolvables(Source_Ref source_r, zypp::Resolvable::Kind kind);

      /** Whether the ResStore is initialized. */
      bool _res_store_initialized;

    private:
      /** Helper indicating creation of nullimpl. */
      struct null {};

      /** Ctor, excl. for nullimpl only.
       * Nullimpl has no Id (\c 0).
      */
      SourceImpl( const null & )
      : base::ProvideNumericId<SourceImpl,Source_Ref::NumericId>( NULL )
      , _res_store_initialized(true)
      {}

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
