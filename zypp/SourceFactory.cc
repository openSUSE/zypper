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

  static media::MediaManager & media_mgr()
  {
	  static media::MediaManager _v;
	  return _v;
  }

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
                                                    const SourceInfo &context )
      {
        Source_Ref::Impl_Ptr impl( new _SourceImpl );
        // note, base_source is a tribool, if indeterminate we fallback to false
        //MIL << "Going to call factory ctor:" << endl;
        //MIL << context << endl;
        impl->factoryCtor( media_r, context.path(), context.alias(), context.cacheDir(), context.baseSource(), context.autorefresh() );
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
    media::MediaId id = media_mgr().open(url_r);
    media_mgr().attach(id);
    Pathname products_file = Pathname("media.1/products");

    try  {
      media_mgr().provideFile (id, products_file);
      products_file = media_mgr().localPath (id, products_file);
      scanProductsFile (products_file, products_r);
    }
    catch ( const Exception & excpt ) {
      ZYPP_CAUGHT(excpt);
      MIL << "No products description found on the Url" << endl;
    }

    media_mgr().release(id);
  }

  Source_Ref SourceFactory::createFrom( const source::SourceInfo &context )
  {
    if ( context.type().empty() )
      {
        return createFrom( context.url(),
                           context.path(),
                           context.alias(),
                           context.cacheDir(),
                           context.baseSource() );
      }
    else
      {
        return createFrom( context.type(),
                           context.url(),
                           context.path(),
                           context.alias(),
                           context.cacheDir(),
                           context.baseSource(),
                           context.autorefresh() );
      }
  }

  template<typename _SourceImpl>
  static bool probeSource(const Url &url_r, const Pathname &path_r, media::MediaId id, const std::string &type, callback::SendReport<ProbeSourceReport> &report )
  {
      boost::function<bool()> probe = typename _SourceImpl::Prober( id, path_r );

      if ( probe() )
      {
        report->successProbe(url_r, type);
        return true;
      }
      else
      {
        report->failedProbe(url_r, type);
        return false;
      }
    return false;
  }

  template<class _SourceImpl>
  Source_Ref SourceFactory::createSourceImplWorkflow( media::MediaId id, const SourceInfo &context )
  {
      //MIL << "Trying (pre) to create source of type " << _SourceImpl::typeString() << endl;
      callback::SendReport<SourceCreateReport> report;
      bool retry = true;
      while (retry)
      {
        report->start( context.url() );
          try
          {
            MIL << "Trying to create source of type " << _SourceImpl::typeString() << endl;
            Source_Ref::Impl_Ptr impl( Impl::createSourceImpl<_SourceImpl>( id, context ) );
            MIL << "Created source " << impl->type() << endl;
            report->finish( context.url(), SourceCreateReport::NO_ERROR, std::string() );
            return Source_Ref(impl);
          }
          catch (const SourceUserRejectedException & excpt_r)
          {
            ZYPP_CAUGHT(excpt_r);
            report->problem( context.url(), SourceCreateReport::REJECTED, "Source rejected by the user" );
            report->finish( context.url(), SourceCreateReport::NO_ERROR, "" );
            ZYPP_THROW(Exception( "Source Rejected: " + excpt_r.asUserString() ));
          }
          catch ( const SourceMetadataException & excpt_r )
          {
            ZYPP_CAUGHT(excpt_r);
            report->problem( context.url(), SourceCreateReport::REJECTED, "Source metadata is invalid: " + excpt_r.asUserString() );
            report->finish( context.url(), SourceCreateReport::REJECTED, ""  );
            ZYPP_THROW(Exception( "Invalid Source: " + excpt_r.asUserString() ));
          }
          catch (const Exception & excpt_r)
          {
            ZYPP_CAUGHT(excpt_r);
            if ( report->problem( context.url(), SourceCreateReport::UNKNOWN, "Unknown Error: " + excpt_r.asUserString() ) != SourceCreateReport::RETRY )
            {
              report->finish( context.url(), SourceCreateReport::UNKNOWN, std::string("Unknown Error: ") + excpt_r.asUserString() );
              ZYPP_THROW(Exception( "Unknown Error: " + excpt_r.asUserString() ));
            }
          }
      }
      // never reached
      return Source_Ref();
  }

  Source_Ref SourceFactory::createFrom( const Url & url_r, const Pathname & path_r, const std::string & alias_r, const Pathname & cache_dir_r, bool base_source )
  {
    if (! url_r.isValid())
      ZYPP_THROW( Exception("Empty URL passed to SourceFactory") );

#warning if cache_dir is provided, no need to open the original url
    // open the media
    media::MediaId id = media_mgr().open(url_r);

    // add dummy verifier
    media_mgr().addVerifier(id, media::MediaVerifierRef(new media::NoVerifier()));
    // attach only if initializing from media and not from cache (#153073)
    if (cache_dir_r == "")
    {
      media_mgr().attach(id);
    }
    else
    {
      MIL << "Initializing from cache" << endl;
    }

    bool auto_refresh = media::MediaAccess::canBeVolatile( url_r );

    SourceInfo context( url_r, path_r, alias_r, cache_dir_r, auto_refresh );
    context.setBaseSource( base_source );

    callback::SendReport<ProbeSourceReport> report;
    bool probeYUM = false;
    bool probeYaST = false;

    report->start(url_r);
    try
    {
      if ( (probeYUM = probeSource<yum::YUMSourceImpl>( url_r, path_r, id, "YUM", report )) )
      {
        // nothing
      }
      else if ( (probeYaST = probeSource<susetags::SuseTagsImpl>( url_r, path_r, id, "YaST", report )) )
      {
        // nohing
      }
      report->finish(url_r, ProbeSourceReport::NO_ERROR, "");

      if ( probeYUM )
      {
        Source_Ref source(createSourceImplWorkflow<source::yum::YUMSourceImpl>( id, context ));
        return source;
      }
      else if ( probeYaST )
      {
        Source_Ref source(createSourceImplWorkflow<susetags::SuseTagsImpl>( id, context ));
        return source;
      }
      else
      {
        ZYPP_THROW( SourceUnknownTypeException("Unknown source type for " + url_r.asString() ) );
      }
    }
    catch ( const Exception &e )
    {
      report->finish(url_r, ProbeSourceReport::IO, e.asUserString());
      ZYPP_RETHROW(e);
    }
    //////////////////////////////////////////////////////////////////
    // TRY PLAINDIR
    //////////////////////////////////////////////////////////////////
    //FIXME disabled

    return Source_Ref(); // not reached!!
  }

  Source_Ref SourceFactory::createFrom( const std::string & type, const Url & url_r, const Pathname & path_r, const std::string & alias_r, const Pathname & cache_dir_r, bool base_source, tribool auto_refresh )
  {
    if (! url_r.isValid())
      ZYPP_THROW( Exception("Empty URL passed to SourceFactory") );

    //callback::SendReport<CreateSourceReport> report;

    //report->startProbe (url_r);

#warning if cache_dir is provided, no need to open the original url
    // open the media
    media::MediaId id = media_mgr().open(url_r);

    // add dummy verifier
    media_mgr().addVerifier(id, media::MediaVerifierRef(new media::NoVerifier()));
    // attach only if initializing from media and not from cache (#153073)
    if (cache_dir_r == "")
    {
      media_mgr().attach(id);
    }
    else
    {
      MIL << "Initializing from cache" << endl;
    }

    bool calculated_autorefresh = auto_refresh;
    // Sane default for unknown autorefresh
    if ( auto_refresh == indeterminate )
      calculated_autorefresh = media::MediaAccess::canBeVolatile( url_r );

    //SourceInfo( url, path, alias, cache_dir, autorefresh );
    SourceInfo context( url_r, path_r, alias_r, cache_dir_r, calculated_autorefresh );
    context.setBaseSource( base_source );
    context.setType( type );

    try
    {
      Source_Ref::Impl_Ptr impl;

      if( type == yum::YUMSourceImpl::typeString() )
      {
        MIL << "Trying the YUM source" << endl;
        impl = Source_Ref::Impl_Ptr( Impl::createSourceImpl<yum::YUMSourceImpl>(id, context ) );
        MIL << "YUM source created" << endl;
      }
      else if ( type == susetags::SuseTagsImpl::typeString() )
      {
        MIL << "Trying the SUSE tags source" << endl;
        impl = Source_Ref::Impl_Ptr( Impl::createSourceImpl<susetags::SuseTagsImpl>(id, context ) );
        MIL << "YaST source created" << endl;
      }
      else if ( type == PlaindirImpl::typeString() )
      {
        MIL << "Trying the Plaindir source" << endl;
        impl = Source_Ref::Impl_Ptr( Impl::createSourceImpl<PlaindirImpl>(id, context ) );
        MIL << "Plaindir source created" << endl;
      }
      else
      {
        ZYPP_THROW( Exception ("Cannot create source of unknown type '" + type + "'"));
      }

      // never reached
      return Source_Ref(impl);
    }
    catch (const Exception & excpt_r)
    {
      ZYPP_CAUGHT(excpt_r);
      MIL << "Creating a source of type " << type << " failed " << endl;
      ZYPP_RETHROW(excpt_r);
    }

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
