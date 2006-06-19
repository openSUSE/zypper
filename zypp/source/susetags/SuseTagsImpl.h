/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsImpl.h
 *
*/
#ifndef ZYPP_SOURCE_SUSETAGS_SUSETAGSIMPL_H
#define ZYPP_SOURCE_SUSETAGS_SUSETAGSIMPL_H

#include <iosfwd>
#include <string>

#include "zypp/Pathname.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/base/DefaultFalseBool.h"
#include "zypp/Product.h"
#include "zypp/CheckSum.h"
#include "zypp/source/susetags/SuseTagsProductImpl.h"
#include "zypp/source/susetags/SuseTagsPackageImpl.h"

using namespace zypp::filesystem;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      struct SuseTagsPackageImplData
      {
        SuseTagsPackageImplData()
        {
        }

        TranslatedText _summary;
        TranslatedText _description;
        TranslatedText _insnotify;
        TranslatedText _delnotify;
        TranslatedText _license_to_confirm;
        std::list<std::string> _authors;
      };

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SuseTagsImpl
      //
      /** */
      class SuseTagsImpl : public SourceImpl
      {
      public:
        typedef intrusive_ptr<SuseTagsImpl>       Ptr;
        typedef intrusive_ptr<const SuseTagsImpl> constPtr;

      public:
        /** Default Ctor.
         * Just initilizes data members. Metadata retrieval
         * is delayed untill \ref factoryInit.
        */
        SuseTagsImpl();
        /** Dtor */
        ~SuseTagsImpl();

      public:
        virtual void createResolvables(Source_Ref source_r);

	/** Provide only resolvables of a certain kind. */
        virtual ResStore provideResolvables(Source_Ref source_r, zypp::Resolvable::Kind kind);

        virtual std::string type(void) const
        { return typeString(); }

        /** Text used for identifying the type of the source.
         * Used by the \ref SourceFactory when creating a
         * source of a given type only.
         */
	static std::string typeString(void)
	{ return "YaST"; }
        virtual Date timestamp() const;
        virtual unsigned numberOfMedia(void) const;
        virtual std::string vendor (void) const;
        virtual const std::list<Pathname> publicKeys();
        virtual std::string unique_id (void) const;

        Pathname sourceDir( const std::string & dir );
        virtual void storeMetadata(const Pathname & cache_dir_r);

        /**
         * Get media verifier for the specified media
         */
        virtual media::MediaVerifierRef verifier(media::MediaNr media_nr);

      protected:
        /** Stream output. */
        virtual std::ostream & dumpOn( std::ostream & str ) const;
        void initCacheDir(const Pathname & cache_dir_r);
        bool cacheExists() const;
      private:
        /** Ctor substitute.
         * Actually get the metadata.
         * \throw EXCEPTION on fail
        */
        virtual void factoryInit();
        
        const Pathname metadataRoot() const;
        const Pathname contentFile() const;
        const Pathname contentFileKey() const;
        const Pathname contentFileSignature() const;
        const Pathname descrDir() const;
        const Pathname mediaFile() const;
        
        void saveMetadataTo(const Pathname & dir_r);
        
        /**
         * description dir in original media
         * it is not always the same so it
         * requires the content file to be parsed
         */
        const Pathname mediaDescrDir() const;
        
        /**
         * data dir in original media
         * it is not always the same so it
         * requires the content file to be parsed
         */
        const Pathname dataDir() const;
        
         /**
         * reads content file, initializes the
         * data and media descr dir
         * and saves the product object
         * but it does not add it to the store yet
          */
        void readContentFile(const Pathname &p);
        
        /**
         * reads a media file and installs
         * a media verifier if available
         */
        void readMediaFile(const Pathname &p);
        
        TmpDir downloadMetadata();
        bool downloadNeeded(const Pathname &localdir);
        
	void provideProducts(Source_Ref source_r, ResStore& store);
	void providePackages(Source_Ref source_r, ResStore& store);
	void provideSelections(Source_Ref source_r, ResStore& store);
        void provideSelection(Source_Ref source_r, ResStore& store);
	void providePatterns(Source_Ref source_r, ResStore& store);

        unsigned _media_count;

        // data dir we are using
        // depends if we are on media or
        // cache
        Pathname _data_dir;

        // descr dir on media.
        // we need it if we refresh
        // already running from cache
        Pathname _media_descr_dir;

        std::string _vendor;
        std::string _media_id;
        /**
         * pointer to the product implementation
         * we need it to access the checksums if we are in verify mode
         */
        detail::ResImplTraits<SuseTagsProductImpl>::Ptr _prodImpl;
        Product::Ptr _product;
        public:

        // shared data between packages with same NVRA
        std::map<NVRA, SuseTagsPackageImplData> _package_data;
        // list of packages which depend on another package for its data
        std::map<NVRA, DefaultFalseBool> _is_shared;
        // list of packages which provide data to another package
        std::map<NVRA, DefaultFalseBool> _provides_shared_data;

        // list of translation files
        std::list<std::string> _pkg_translations;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////

    using susetags::SuseTagsImpl;

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SUSETAGS_SUSETAGSIMPL_H
