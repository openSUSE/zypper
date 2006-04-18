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
#include "zypp/Product.h"
#include "zypp/source/susetags/SuseTagsProductImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

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
        bool cacheExists();
      private:
        /** Ctor substitute.
         * Actually get the metadata.
         * \throw EXCEPTION on fail
        */
        virtual void factoryInit();
	
        void readContentFile();
        
	void provideProducts(Source_Ref source_r, ResStore& store);
	void providePackages(Source_Ref source_r, ResStore& store);
	void provideSelections(Source_Ref source_r, ResStore& store);
        void provideSelection(Source_Ref source_r, ResStore& store);
	void providePatterns(Source_Ref source_r, ResStore& store);
	
         /**
         * verify media mode (use the new META tags)
          */
        bool verifyChecksumsMode();
        
        /** 
         * Verify file checksum
         * \throw EXCEPTION on verification file
         */
        void verifyFile( const Pathname &path, const std::string &key);
        
        unsigned _media_count;
	
        // descr dir we are using
        // depends if we are on media or
        // cache
        Pathname _descr_dir;
        Pathname _data_dir;
        
        // descr dir on media.
        // we need it if we refresh
        // already running from cache
        Pathname _orig_descr_dir;
        
        Pathname _content_file;
        Pathname _content_file_sig;
        Pathname _content_file_key;
        
        std::string _vendor;
        std::string _media_id;
        /**
         * pointer to the product implementation
         * we need it to access the checksums if we are in verify mode
         */
        detail::ResImplTraits<SuseTagsProductImpl>::Ptr _prodImpl;
        Product::Ptr _product;
       
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
