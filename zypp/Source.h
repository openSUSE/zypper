/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Source.h
 *
*/
#ifndef ZYPP_SOURCE_H
#define ZYPP_SOURCE_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Logger.h"

#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/Resolvable.h"

#include "zypp/media/MediaManager.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace source
  {
    class SourceImpl;
    DEFINE_PTR_TYPE(SourceImpl);
  }
  class ResStore;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Source
  //
  /**
   * \note Source is a reference to the implementation. No COW
   * is performed.
  */
  class Source_Ref
  {
    friend std::ostream & operator<<( std::ostream & str, const Source_Ref & obj );
    friend bool operator==( const Source_Ref & lhs, const Source_Ref & rhs );
    friend bool operator<( const Source_Ref & lhs, const Source_Ref & rhs );

  public:
    typedef source::SourceImpl     Impl;
    typedef source::SourceImpl_Ptr Impl_Ptr;

  public:

    /** Default ctor: noSource.
     * Real Sources are to be created via SourceFactory.
    */
    Source_Ref();

    /** A dummy Source providing nothing, doing nothing.
     * \todo provide a _constRef
    */
    static const Source_Ref noSource;

  public:

    /** All resolvables provided by this source. */
    const ResStore & resolvables() const;

    /** All resolvables of a given kind provided by this source. */
    const ResStore resolvables(zypp::Resolvable::Kind kind) const;

    /** Provide a file to local filesystem */
    const Pathname provideFile(const Pathname & file_r,
			       const unsigned media_nr = 1);
    const Pathname provideDir(const Pathname & dir_r,
		              const unsigned media_nr = 1,
			      const bool recursive = false);

    const bool enabled() const;

    void enable();

    void disable();

    const bool autorefresh() const;
    void setAutorefresh( const bool enable_r );
    void refresh();

    void storeMetadata(const Pathname & cache_dir_r);

    std::string alias (void) const;
    
    // string description of the source type, e.g. "YUM" or "UnitedLinux"
    std::string type (void) const;

    unsigned numberOfMedia(void) const;

    std::string vendor (void) const;

    std::string unique_id (void) const;

    // generic information get/set
    std::string id (void) const;
    void setId (const std::string id_r);
    unsigned priority (void) const;
    void setPriority (unsigned p);
    unsigned priorityUnsubscribed (void) const;
    void setPriorityUnsubscribed (unsigned p);
    const Pathname & cacheDir (void) const;
    const std::set<Pathname> publicKeys() const;
    
    // for ZMD
    std::string zmdName (void) const;
    void setZmdName (const std::string name_r);
    std::string zmdDescription (void) const;
    void setZmdDescription (const std::string desc_r);

    // for YaST
    Url url (void) const;
    const Pathname & path (void) const;

  public:
    /**
     * Change the media of the source (in case original media is not available)
     * The media must be ready-to-use (in the same form as when passing to SourceImpl constructor)
     */
    void changeMedia(const media::MediaId & media_r, const Pathname & path_r);
    
    /**
     * Redirect the given media to the given URL instead of the standard one.
     */
    void redirect(unsigned media_nr, const Url & new_url);

    /**
     * Release all medias attached by the source
     */
    void release();

    /**
     * Reattach the source if it is not mounted, but downloaded,
     * to different directory
     *
     * \throws Exception
     */
    void reattach(const Pathname &attach_point);

    /**
     * Provide a media verifier suitable for the given media number
     */
    media::MediaVerifierRef verifier(unsigned media_nr);

    /** Conversion to bool to allow pointer style tests
     *  for nonNULL \ref source impl.
     * \todo fix by providing a safebool basecalss, doing the 'nasty'
     * things.
    */
    // see http://www.c-plusplus.de/forum/viewtopic-var-t-is-113762-and-start-is-0-and-postdays-is-0-and-postorder-is-asc-and-highlight-is-.html
    // for the glory details

    typedef void (Source_Ref::*unspecified_bool_type)();

    operator unspecified_bool_type() const
    {
      if ( *this == noSource )
        return static_cast<unspecified_bool_type>(0);
      return &Source_Ref::enable;	// return pointer to a void() function since the typedef is like this
    }

  private:
    /** Factory */
    friend class SourceFactory;
    friend class source::SourceImpl;

  private:
    /** Factory ctor */
    explicit
    Source_Ref( const Impl_Ptr & impl_r );

  private:
    /** Pointer to implementation */
    Impl_Ptr _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Source Stream output. */
  std::ostream & operator<<( std::ostream & str, const Source_Ref & obj );

  /** \relates Source_Ref Equal if same implementation class. */
  inline bool operator==( const Source_Ref & lhs, const Source_Ref & rhs )
  { return lhs._pimpl == rhs._pimpl; }

  /** \relates Source_Ref Order in std::conainer based on _pimpl. */
  inline bool operator<( const Source_Ref & lhs, const Source_Ref & rhs )
  { return lhs._pimpl < rhs._pimpl; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_H
