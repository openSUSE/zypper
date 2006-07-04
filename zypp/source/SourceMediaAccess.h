/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SourceMediaAccess_H
#define ZYPP_SourceMediaAccess_H

#include <iosfwd>
#include <string>
#include <vector>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/media/MediaManager.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(SourceMediaAccess);


    class SourceMediaVerifier : public zypp::media::MediaVerifierBase
    {
      public:
      /** ctor */
      SourceMediaVerifier(const std::string & vendor_r, const std::string & id_r, const media::MediaNr media_nr = 1);
      /**
       * Check if the specified attached media contains
       * the desired media number (e.g. SLES10 CD1).
       */
      virtual bool isDesiredMedia(const media::MediaAccessRef &ref);
      private:
        std::string _media_vendor;
        std::string _media_id;
        media::MediaNr _media_nr;
    };

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SourceCache
    //
    class SourceMediaAccess : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const SourceMediaAccess & obj );

    public:
      SourceMediaAccess( const Url &url, const Pathname &path, std::vector<media::MediaVerifierRef> verifiers );
      ~SourceMediaAccess();
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      Url _url;
      Pathname _path;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates SourceMediaAccess Stream output */
    inline std::ostream & operator<<( std::ostream & str, const SourceMediaAccess & obj )
    { return obj.dumpOn( str ); }


    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SourceMediaAccess_H
