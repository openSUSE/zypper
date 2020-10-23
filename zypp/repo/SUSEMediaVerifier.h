/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SUSE_MEDIAVERIFIER_H
#define ZYPP_SUSE_MEDIAVERIFIER_H

#include <iosfwd>

#include <zypp/media/MediaManager.h>
#include <zypp/media/MediaHandler.h>
#include <zypp/base/PtrTypes.h>

namespace zypp
{
  namespace repo
  {
    ///////////////////////////////////////////////////////////////////
    ///
    /// \short Implementation of the traditional SUSE media verifier
    ///
    class SUSEMediaVerifier : public media::MediaVerifierBase
    {
    public:
      /** Ctor creating a verifier by parsing media file
       *
       * \param path_r Path to media.1/media kind file
       */
      explicit SUSEMediaVerifier( const Pathname & path_r, media::MediaNr mediaNr_r = 1 );

      /** Ctor cloning a verifier for a different \a mediaNr_r
       *
       * \param path_r Path to media.1/media kind file
       */
      SUSEMediaVerifier( const SUSEMediaVerifier & rhs, media::MediaNr mediaNr_r );

         /** Dtor */
      ~SUSEMediaVerifier() override;

    public:
      /** Validate object in a boolean context: valid */
      explicit operator bool() const
      { return valid(); }

      /** Data considered to be valid if we have vendor and ident. */
      bool valid() const;

       /** Medias expected vendor string. */
      const std::string & vendor() const;

      /** Medias expected ident string. */
      const std::string & ident() const;

      /** The total number of media in this set (or 0 if not known). */
      media::MediaNr totalMedia() const;

      /** Media number expected by this verifier (starts with 1). */
      media::MediaNr mediaNr() const;

    public:
      /** Check if \ref_r accesses the desired media.
       *
       * The check is optimistic. If we can't get reliable data from the server,
       * we nevertheless assume the media is valid. File downloads will fail if
       * this was not true.
       */
      bool isDesiredMedia( const media::MediaHandler & ref_r ) const override;

    public:
      class Impl;                 ///< Implementation class.
      friend std::ostream & operator<<( std::ostream & str, const SUSEMediaVerifier & obj );
    private:
      RW_pointer<Impl> _pimpl; ///< Pointer to implementation.
    };

    /** \relates SUSEMediaVerifier Stream output  */
    //std::ostream & operator<<( std::ostream & str, const SUSEMediaVerifier & obj );

  } // namespace repo
} // namespace zypp
#endif
