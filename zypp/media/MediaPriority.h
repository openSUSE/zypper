/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/media/MediaPriority.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIAPRIORITY_H
#define ZYPP_MEDIA_MEDIAPRIORITY_H

#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Url;

  ///////////////////////////////////////////////////////////////////
  namespace media
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaPriority
    //
    /** Derive a numeric priority from \ref Url scheme according to zypp.conf(download.media_preference).
     *
     * The class is simple. Constructable and assignable from \ref Url
     * or scheme string. Implicit convertible into a numic \ref value_type.
     *
     * \code
     *   if ( MediaPriority("cd") \< MediaPriority("ftp") )
     *     ...
     * \endcode
     *
     * \todo Maybe introduce a static tribool, to allow overwriting zypp.conf(download.media_preference) default.
    */
    class MediaPriority
    {
      public:
	typedef int value_type;

      public:
	/** Default ctor. Least priority \c 0.*/
	MediaPriority()
	: _val( 0 )
	{}

	/** Copy ctor. */
	MediaPriority( value_type val_r )
	: _val( val_r )
	{}

	/** Ctor from scheme string.*/
	MediaPriority( const std::string & scheme_r );

	/** Ctor from scheme string defined by Url. */
	MediaPriority( const Url & url_r );

      public:
	/** Assign. */
	MediaPriority & operator=( value_type rhs )
	{ _val = rhs; return *this; }

	/** Assign priority of scheme string. */
	MediaPriority & operator=( const std::string & scheme_r )
	{ _val = MediaPriority(scheme_r); return *this; }

	/** Assign priority of scheme string defined by Url. */
	MediaPriority & operator=( const Url & url_r )
	{ _val = MediaPriority(url_r); return *this; }

      public:
	/** Conversion to value_type. */
	//@{
	/** Explicit */
	value_type &       get()            { return _val; }
	/** Explicit */
	const value_type & get() const      { return _val; }
	/** Implicit */
	operator value_type &()             { return get(); }
	/** Implicit */
	operator const value_type &() const { return get(); }
	//@}

      private:
	value_type _val;
    };

    /////////////////////////////////////////////////////////////////
  } // namespace media
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MEDIA_MEDIAPRIORITY_H
