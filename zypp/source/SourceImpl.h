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
#include "zypp/Source.h"

#include "zypp/ResStore.h"

#include "zypp/Pathname.h"
#include "zypp/media/MediaManager.h"

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
     * Constructed by \ref SourceFactory. Public access via \ref Source
     * interface.
    */
    class SourceImpl : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const SourceImpl & obj );

    public:
      /** Ctor, FIXME it is here only because of target storage, then make it private */
      SourceImpl()
      : _res_store_initialized(true) // in case of null source, nothing to read
      {}

      /** Ctor. */
      SourceImpl(media::MediaId & media_r,
                 const Pathname & path_r = "/",
		 const std::string & alias = "");
      /** Dtor. */
      virtual ~SourceImpl();

    public:

      /** All resolvables provided by this source. */
      const ResStore & resolvables(Source_Ref source_r) const;

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
      const Pathname provideDir(const Pathname & path,
				const unsigned media_nr = 1,
				const bool recursive = false);

      const bool enabled() const
      { return _enabled; }

      void enable()
      { _enabled = true; }

      void disable()
      { _enabled = false; }

      std::string alias (void) const
      { return _alias; }

      virtual std::string zmdname (void) const;
      virtual std::string zmddescription (void) const;
      virtual unsigned priority (void) const;
      virtual unsigned priority_unsubscribed (void) const;

      Url url (void) const;

      const Pathname & path (void) const;

      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;

    protected:
      /** All resolvables provided by this source. */
      ResStore _store;
      /** Handle to media which contains this source */
      media::MediaId _media;
      /** Path to the source on the media */
      Pathname _path;
      /** The source is enabled */
      bool _enabled;
      /** (user defined) alias of the source */
      std::string _alias;
      /** (user defined) default priority of the source */
      unsigned _priority;
      /** (user defined) unsubscribed priority of the source */
      unsigned _priority_unsubscribed;

    private:
      /**  */
      /** Late initialize the ResStore. */
      virtual void createResolvables(Source_Ref source_r);
      /** Whether the ResStore is initialized. */
      bool _res_store_initialized;

    public:
      /** Offer default Impl. */
      static SourceImpl_Ptr nullimpl()
      {
        static SourceImpl_Ptr _nullimpl( new SourceImpl );
        return _nullimpl;
      }

      class Verifier : public media::MediaVerifierBase
      {
      public:
	/** ctor */
	Verifier (const std::string & vendor_r, const std::string & id_r);
	/*
	 ** Check if the specified attached media contains
	 ** the desired media number (e.g. SLES10 CD1).
	 */
	virtual bool
	isDesiredMedia(const media::MediaAccessRef &ref, media::MediaNr mediaNr);

      private:
	std::string _media_vendor;
	std::string _media_id;
	SourceImpl_Ptr _source;
      };


    };
    ///////////////////////////////////////////////////////////////////

    /** \relates SourceImpl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const SourceImpl & obj )
    { return obj.dumpOn( str ); }

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SOURCEIMPL_H
