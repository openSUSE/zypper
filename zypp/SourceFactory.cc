/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceFactory.cc
 *
*/
#include <iostream>
#include <fstream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/String.h"

#include "zypp/SourceFactory.h"
#include "zypp/source/Builtin.h"
#include "zypp/media/MediaAccess.h"
#include "zypp/ZYppCallbacks.h"

using std::endl;
using namespace zypp::source;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////


media::MediaManager media_mgr;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceFactory::Impl
  //
  /** SourceFactory implementation. */
  struct SourceFactory::Impl
  {
    /** Try to create a \a _SourceImpl kind of Source.
     * \throw EXCEPTION if creation fails
    */
    template<class _SourceImpl>
      static Source_Ref::Impl_Ptr createSourceImpl( const media::MediaId & media_r,
                                                    const Pathname & path_r,
                                                    const std::string & alias_r,
                                                    const Pathname & cache_dir_r )
      {
        Source_Ref::Impl_Ptr impl( new _SourceImpl );
        impl->factoryCtor( media_r, path_r, alias_r, cache_dir_r );
        return impl;
      }


  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceFactory
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceFactory::SourceFactory
  //	METHOD TYPE : Ctor
  //
  SourceFactory::SourceFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceFactory::~SourceFactory
  //	METHOD TYPE : Dtor
  //
  SourceFactory::~SourceFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceFactory::createFrom
  //	METHOD TYPE : Source
  //
  Source_Ref SourceFactory::createFrom( const Source_Ref::Impl_Ptr & impl_r )
  {
    return impl_r ? Source_Ref( impl_r ) : Source_Ref::noSource;
  }

  void SourceFactory::listProducts( const Url & url_r, ProductSet & products_r )
  {
    if (! url_r.isValid())
      ZYPP_THROW( Exception("Empty URL passed to SourceFactory") );

    // open the media
    media::MediaId id = media_mgr.open(url_r);
    media_mgr.attach(id);
    Pathname products_file = Pathname("media.1/products");
    media_mgr.provideFile (id, 1, products_file);
    products_file = media_mgr.localPath (id, products_file);
    scanProductsFile (products_file, products_r);
    media_mgr.release(id);
  }

  Source_Ref SourceFactory::createFrom( const Url & url_r, const Pathname & path_r, const std::string & alias_r, const Pathname & cache_dir_r )
  {
    if (! url_r.isValid())
      ZYPP_THROW( Exception("Empty URL passed to SourceFactory") );

    callback::SendReport<CreateSourceReport> report;

    report->startProbe (url_r);
    
    // open the media
    media::MediaId id = media_mgr.open(url_r);

    // add dummy verifier
    media_mgr.addVerifier(id, media::MediaVerifierRef(new media::NoVerifier()));
    media_mgr.attach(id);
    try
    {
      MIL << "Trying the YUM source" << endl;
      Source_Ref::Impl_Ptr impl( Impl::createSourceImpl<yum::YUMSourceImpl>(id, path_r, alias_r, cache_dir_r) );
      MIL << "Found the YUM source" << endl;
      
      report->endProbe (url_r);
      
      return Source_Ref(impl);
    }
    catch (const Exception & excpt_r)
    {
      ZYPP_CAUGHT(excpt_r);
      MIL << "Not YUM source, trying next type" << endl;
    }
    try
    {
      MIL << "Trying the SUSE tags source" << endl;
#warning TODO pass cache_dir_r once constructor adapted
      Source_Ref::Impl_Ptr impl( Impl::createSourceImpl<susetags::SuseTagsImpl>(id, path_r, alias_r, cache_dir_r) );
      MIL << "Found the SUSE tags source" << endl;
      
      report->endProbe (url_r);

      return Source_Ref(impl);
    }
    catch (const Exception & excpt_r)
    {
      ZYPP_CAUGHT(excpt_r);
      MIL << "Not SUSE tags source, trying next type" << endl;
    }
    
    report->endProbe (url_r);

    ERR << "No next type of source" << endl;
    ZYPP_THROW(Exception("Cannot create the installation source"));
    return Source_Ref(); // not reached!!
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SourceFactory & obj )
  {
    return str << "SourceFactory";
  }

  void SourceFactory::scanProductsFile( const Pathname & file_r, ProductSet & pset_r ) const
  {
    std::ifstream pfile( file_r.asString().c_str() );
    while ( pfile.good() ) {

      std::string value = str::getline( pfile, str::TRIM );
      if ( pfile.bad() ) {
        ERR << "Error parsing " << file_r << endl;
        ZYPP_THROW(Exception("Error parsing " + file_r.asString()));
      }
      if ( pfile.fail() ) {
        break; // no data on last line
      }
      std::string tag = str::stripFirstWord( value, true );

      if ( tag.size() ) {
        pset_r.insert( ProductEntry( tag, value ) );
      }
    }
  }



  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
