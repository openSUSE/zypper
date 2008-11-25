/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Fd.h
 *
*/
#ifndef ZYPP_BASE_FD_H
#define ZYPP_BASE_FD_H

#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Fd
    //
    /** Assert \c close called on open filedescriptor.
     * \code
     * ...
     * scoped_ptr<Fd> fd; // calls close when going out of scope
     * try {
     *   fd.reset( new Fd( "/some/file" ) );
     * } catch ( ... ) {
     *   // open failed.
     * }
     * read( fd->fd(), ... ),
     * \endcode
     *
     * \ingroup g_RAII
     * \todo It's dumb. Openflags and more related functions (read/write..)
     * could be added.
    */
    class Fd
    {
    public:
      /** Ctor opens file.
       * \throw EXCEPTION If open fails.
      */
      Fd( const Pathname & file_r, int open_flags, mode_t mode = 0 );

      /** Dtor closes file. */
      ~Fd()
      { close(); }

      /** Explicitly close the file. */
      void close();

      /** Test for valid filedescriptor. */
      bool isOpen() const
      { return m_fd != -1; }

      /** Return the filedescriptor. */
      int fd() const
      { return m_fd; }

    private:
      /** The filedescriptor. */
      int m_fd;
      /** No copy. */
      Fd( const Fd & );
      /** No assign. */
      Fd & operator=( const Fd & );
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_FD_H
