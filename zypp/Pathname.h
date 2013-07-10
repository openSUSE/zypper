/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Pathname.h
 *
*/
#ifndef ZYPP_PATHNAME_H
#define ZYPP_PATHNAME_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Url;

  ///////////////////////////////////////////////////////////////////
  namespace filesystem
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Pathname
    //
    /** Pathname.
     *
     * \note For convenience Pathname is available as zypp::Pathname too.
     *
     * Always stores normalized paths (no inner '.' or '..' components
     * and no consecutive '/'es). Concatenation automatically adds
     * the path separator '/'.
     *
     * \todo Add support for handling extensions incl. stripping
     * extensions from basename (basename("/path/foo.baa", ".baa") ==> "foo")
    */
    class Pathname
    {
    public:
      /** Default ctor: an empty path. */
      Pathname()
      {}

      /** Ctor from string. */
      Pathname( const std::string & name_r )
      { _assign( name_r ); }

      /** Ctor from char*. */
      Pathname( const char * name_r )
      { _assign( name_r ? name_r : "" ); }

      /** Copy Ctor */
      Pathname( const Pathname & rhs )
      : _name( rhs._name )
      {}

      /** Swap */
      friend void swap( Pathname & lhs, Pathname & rhs )
      {
	using std::swap;
	swap( lhs._name, rhs._name );
      }
#ifndef SWIG // Swig treats it as syntax error
      /** Move Ctor */
      Pathname( Pathname && tmp )
      : _name( std::move( tmp._name ) )
      {}
#endif
      /** Assign */
      Pathname & operator=( Pathname rhs )
      { swap( *this, rhs ); return *this; }

      /** Concatenate and assing. \see cat */
      Pathname & operator/=( const Pathname & path_tv )
      { return( *this = cat( *this, path_tv ) ); }

      /** Concatenate and assing. \see cat
       * \deprecated: use /=
      */
      Pathname & operator+=( const Pathname & path_tv )
      { return( *this = cat( *this, path_tv ) ); }

      /** String representation. */
      const std::string & asString() const
      { return _name; }

      /** String representation as "(root)/path" */
      static std::string showRoot( const Pathname & root_r, const Pathname & path_r );

      /** String representation as "(root)/path", unless \a root is \c "/" or empty. */
      static std::string showRootIf( const Pathname & root_r, const Pathname & path_r );

      /** Url representation using \c scheme_r schema . */
      Url asUrl( const std::string & scheme_r ) const;
      /** \overload using \c dir schema. */
      Url asUrl() const;
      /** \overload using \c dir schema. */
      Url asDirUrl() const;
      /** \overload using \c file schema. */
      Url asFileUrl() const;

      /** String representation. */
      const char * c_str() const
      { return _name.c_str(); }

      /** Test for an empty path. */
      bool empty()    const { return _name.empty(); }
      /** Test for an absolute path. */
      bool absolute() const { return *_name.c_str() == '/'; }
      /** Test for a relative path. */
      bool relative() const { return !( absolute() || empty() ); }

      /** Return all but the last component od this path. */
      Pathname dirname() const { return dirname( *this ); }
      static Pathname dirname( const Pathname & name_r );

      /** Return the last component of this path. */
      std::string basename() const { return basename( *this ); }
      static std::string basename( const Pathname & name_r );

      /** Return all of the characters in name after and including
       * the last dot in the last element of name.  If there is no dot
       * in the last element of name then returns the empty string.
      */
      std::string extension() const { return extension( *this ); }
      static std::string extension( const Pathname & name_r );

      /** Return this path, adding a leading '/' if relative. */
      Pathname absolutename() const { return absolutename( *this ); }
      static Pathname absolutename( const Pathname & name_r )
      { return name_r.relative() ? cat( "/", name_r ) : name_r; }

      /** Return this path, removing a leading '/' if absolute.*/
      Pathname relativename() const { return relativename( *this ); }
      static Pathname relativename( const Pathname & name_r )
      { return name_r.absolute() ? cat( ".", name_r ) : name_r; }

      /** Return \c path_r prefixed with \c root_r, unless it is already prefixed. */
      static Pathname assertprefix( const Pathname & root_r, const Pathname & path_r );

      /** Concatenation of pathnames.
       * \code
       *   "foo"  / "baa"  ==> "foo/baa"
       *   "foo/" / "baa"  ==> "foo/baa"
       *   "foo"  / "/baa" ==> "foo/baa"
       *   "foo/" / "/baa" ==> "foo/baa"
       * \endcode
      */
      Pathname cat( const Pathname & r ) const { return cat( *this, r ); }
      static Pathname cat( const Pathname & l, const Pathname & r );

      /** Append string \a r to the last component of the path.
       * \code
       *   "foo/baa".extend( ".h" ) ==> "foo/baa.h"
       * \endcode
      */
      Pathname extend( const std::string & r ) const { return extend( *this, r ); }
      static Pathname extend( const Pathname & l, const std::string & r );

    private:
      std::string _name;
      void _assign( const std::string & name_r );
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Pathname */
    inline bool operator==( const Pathname & l, const Pathname & r )
    { return l.asString() == r.asString(); }

    /** \relates Pathname */
    inline bool operator!=( const Pathname & l, const Pathname & r )
    { return l.asString() != r.asString(); }

    /** \relates Pathname Concatenate two Pathname. */
    inline Pathname operator/( const Pathname & l, const Pathname & r )
    { return Pathname::cat( l, r ); }

    /** \relates Pathname Concatenate two Pathname.
     * \deprecated: use /
    */
    inline Pathname operator+( const Pathname & l, const Pathname & r )
    { return Pathname::cat( l, r ); }

    /** \relates Pathname */
    inline bool operator<( const Pathname & l, const Pathname & r )
    { return l.asString() < r.asString(); }

    ///////////////////////////////////////////////////////////////////

    /** \relates Pathname Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Pathname & obj )
    { return str << obj.asString(); }

    /////////////////////////////////////////////////////////////////
  } // namespace filesystem
  ///////////////////////////////////////////////////////////////////

  /** Dragged into namespace zypp. */
  using filesystem::Pathname;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATHNAME_H
