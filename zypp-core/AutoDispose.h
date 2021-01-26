/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/AutoDispose.h
 *
*/
#ifndef ZYPP_AUTODISPOSE_H
#define ZYPP_AUTODISPOSE_H

#include <iosfwd>
#include <boost/call_traits.hpp>

#include <zypp-core/base/NonCopyable.h>
#include <zypp-core/base/PtrTypes.h>
#include <zypp-core/base/Function.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : AutoDispose<Tp>
  //
  /** Reference counted access to a \c Tp object calling a custom
   *  \c Dispose function when the last AutoDispose handle to it is
   *  destroyed or reset.
   *
   * \note As with pointers, constness of an \c AutoDispose object does
   * \b not apply to the stored \c Tp object. If the stored \c Tp object
   * should be immutable, you should use <tt>AutoDispose\<const Tp\></tt>.
   *
   * Pass a filename to the application and provide the appropriate
   * code to be executed when the file is no longer needed:
   * \code
   * struct FileCache
   * {
   *   Pathname getFile();
   *   void     releaseFile( const Pathname & );
   * };
   *
   * static FileCache cache;
   *
   * void unlink( const Pathname & file_r );
   *
   * AutoDispose<const Pathname> provideFile( ... )
   * {
   *   if ( file_is_in_cache )
   *     {
   *       // will call 'cache.releaseFile( file )'
   *       return AutoDispose<const Pathname>( cache.getFile(),
   *                                           bind( &FileCache::releaseFile, ref(cache), _1 ) );
   *     }
   *   else if ( file_is_temporary )
   *     {
   *       // will call 'unlink( file )'
   *       return AutoDispose<const Pathname>( file, unlink );
   *     }
   *   else if ( file_is_permanent )
   *     {
   *       // will do nothing.
   *       return AutoDispose<const Pathname>( file );
   *     }
   *   else
   *     {
   *       // will do nothing.
   *       return AutoDispose<const Pathname>();
   *     }
   * }
   * \endcode
   *
   * Exception safe handling of temporary files:
   * \code
   * void provideFileAt( const Pathname & destination )
   * {
   *   AutoDispose<const Pathname> guard( destination, unlink );
   *
   *   // Any exception here will lead to 'unlink( destination )'
   *   // ...
   *
   *   // On success: reset the dispose function to NOOP.
   *   guard.resetDispose();
   * }
   * \endcode
  */
  template<class Tp>
    class AutoDispose
    {
    public:
      typedef typename boost::call_traits<Tp>::param_type       param_type;
      typedef typename boost::call_traits<Tp>::reference        reference;
      typedef typename boost::call_traits<Tp>::const_reference  const_reference;
      typedef Tp                                                value_type;
      typedef typename boost::call_traits<Tp>::value_type       result_type;

    public:
      /** Dispose function signatue. */
      typedef function<void ( param_type )> Dispose;

    public:
      /** Default Ctor using default constructed value and no dispose function. */
      AutoDispose()
      : _pimpl( new Impl( value_type() ) )
      {}

      /** Ctor taking dispose function and using default constructed value. */
      explicit AutoDispose( const Dispose & dispose_r )
      : _pimpl( new Impl( value_type(), dispose_r ) )
      {}

      /** Ctor taking value and no dispose function. */
      explicit AutoDispose( param_type value_r )
      : _pimpl( new Impl( value_r ) )
      {}

      /** Ctor taking value and dispose function. */
      AutoDispose( param_type value_r, const Dispose & dispose_r )
      : _pimpl( new Impl( value_r, dispose_r ) )
      {}

    public:

      /** Provide implicit conversion to \c Tp\&. */
      operator reference() const
      { return _pimpl->_value; }

      /** Reference to the \c Tp object. */
      reference value() const
      { return _pimpl->_value; }

      /** Reference to the \c Tp object. */
      reference operator*() const
      { return _pimpl->_value; }

      /** Pointer to the \c Tp object (asserted to be <tt>!= NULL</tt>). */
      value_type * operator->() const
      { return & _pimpl->_value; }

      /** Reset to default Ctor values. */
      void reset()
      { AutoDispose().swap( *this ); }

      /** Exchange the contents of two AutoDispose objects. */
      void swap( AutoDispose & rhs )
      { _pimpl.swap( rhs._pimpl ); }

    public:
      /** Return the current dispose function. */
      const Dispose & getDispose() const
      { return _pimpl->_dispose; }

      /** Set a new dispose function. */
      void setDispose( const Dispose & dispose_r )
      { _pimpl->_dispose = dispose_r; }

      /** Set no dispose function. */
      void resetDispose()
      { setDispose( Dispose() ); }

      /** Exchange the dispose function. */
      void swapDispose( Dispose & dispose_r )
      { _pimpl->_dispose.swap( dispose_r ); }

    private:
      struct Impl : private base::NonCopyable
      {
        Impl( param_type value_r )
        : _value( value_r )
        {}
        Impl( param_type value_r, const Dispose & dispose_r )
        : _value( value_r )
        , _dispose( dispose_r )
        {}
        ~Impl()
        {
          if ( _dispose )
            try { _dispose( _value ); } catch(...) {}
        }
        value_type _value;
        Dispose    _dispose;
      };

      shared_ptr<Impl> _pimpl;
    };

    template<>
    class AutoDispose<void>
    {
    public:
      /** Dispose function signatue. */
      typedef function<void ()> Dispose;

    public:
      /** Default Ctor using default constructed value and no dispose function. */
      AutoDispose()
        : _pimpl( new Impl() )
      {}

      /** Ctor taking dispose function and using default constructed value. */
      explicit AutoDispose( const Dispose & dispose_r )
        : _pimpl( new Impl( dispose_r ) )
      {}

    public:

      /** Reset to default Ctor values. */
      void reset()
      { AutoDispose().swap( *this ); }

      /** Exchange the contents of two AutoDispose objects. */
      void swap( AutoDispose & rhs )
      { _pimpl.swap( rhs._pimpl ); }

    public:
      /** Return the current dispose function. */
      const Dispose & getDispose() const
      { return _pimpl->_dispose; }

      /** Set a new dispose function. */
      void setDispose( const Dispose & dispose_r )
      { _pimpl->_dispose = dispose_r; }

      /** Set no dispose function. */
      void resetDispose()
      { setDispose( Dispose() ); }

      /** Exchange the dispose function. */
      void swapDispose( Dispose & dispose_r )
      { _pimpl->_dispose.swap( dispose_r ); }

    private:
      struct Impl : private base::NonCopyable
      {
        Impl( )
        {}

        Impl( const Dispose & dispose_r )
          : _dispose( dispose_r )
        {}

        ~Impl()
        {
          if ( _dispose )
            try { _dispose(); } catch(...) {}
        }
        Dispose    _dispose;
      };
      shared_ptr<Impl> _pimpl;
    };

  /*!
   * Simple way to run a function at scope exit:
   * \code
   * bool wasBlocking = unblockFile( fd, true );
   * OnScopeExit cleanup( [wasBlocking, fd](){
   *  if ( wasBlocking ) unblockFile( fd, false );
   * });
   * \endcode
   */
  using OnScopeExit = AutoDispose<void>;

  ///////////////////////////////////////////////////////////////////

  /** \relates AutoDispose Stream output of the \c Tp object. */
  template<class Tp>
    inline std::ostream & operator<<( std::ostream & str, const AutoDispose<Tp> & obj )
    { return str << obj.value(); }


  ///////////////////////////////////////////////////////////////////
  /// \class AutoFD
  /// \brief \ref AutoDispose\<int>  calling \c ::close
  /// \ingroup g_RAII
  ///////////////////////////////////////////////////////////////////
  struct AutoFD : public AutoDispose<int>
  {
    AutoFD( int fd_r = -1 ) : AutoDispose<int>( fd_r, [] ( int fd_r ) { if ( fd_r != -1 ) ::close( fd_r ); } ) {}
  };

  ///////////////////////////////////////////////////////////////////
  /// \class AutoFILE
  /// \brief \ref AutoDispose\<FILE*> calling \c ::fclose
  /// \see \ref AutoDispose
  /// \ingroup g_RAII
  ///////////////////////////////////////////////////////////////////
  struct AutoFILE : public AutoDispose<FILE*>
  {
    AutoFILE( FILE* file_r = nullptr ) : AutoDispose<FILE*>( file_r, [] ( FILE* file_r ) { if ( file_r ) ::fclose( file_r ); } ) {}
  };

  ///////////////////////////////////////////////////////////////////
  /// \class AutoFREE<Tp>
  /// \brief \ref AutoDispose\<Tp*> calling \c ::free
  /// \ingroup g_RAII
  ///////////////////////////////////////////////////////////////////
  template <typename Tp>
  struct AutoFREE : public AutoDispose<Tp*>
  {
    AutoFREE( Tp* ptr_r = nullptr ) : AutoDispose<Tp*>( ptr_r, [] ( Tp* ptr_r ) { if ( ptr_r ) ::free( ptr_r ); } ) {}
    AutoFREE( void* ptr_r ) : AutoFREE( static_cast<Tp*>(ptr_r) ) {}
  };

  template <>
  struct AutoFREE<void> : public AutoDispose<void*>
  {
    AutoFREE( void* ptr_r = nullptr ) : AutoDispose<void*>( ptr_r, [] ( void* ptr_r ) { if ( ptr_r ) ::free( ptr_r ); } ) {}
  };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_AUTODISPOSE_H
