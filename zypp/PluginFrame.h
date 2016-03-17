/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PluginFrame.h
 *
*/
#ifndef ZYPP_PLUGINFRAME_H
#define ZYPP_PLUGINFRAME_H

#include <iosfwd>
#include <string>
#include <map>

#include "zypp/base/PtrTypes.h"

#include "zypp/PluginFrameException.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Command frame for communication with \ref PluginScript
   *
   * \code
   *   COMMAND
   *   key:value header lines
   *
   *   multiline body separated from header
   *   by an empty line and terminated by NUL.
   *   ^@
   * \endcode
   *
   * \see PluginScript
   */
  class PluginFrame
  {
    friend std::ostream & operator<<( std::ostream & str, const PluginFrame & obj );
    friend bool operator==( const PluginFrame & lhs, const PluginFrame & rhs );

    typedef const std::initializer_list<std::pair<std::string,std::string>> & HeaderInitializerList;

    public:
      /** "ACK" command. */
      static const std::string & ackCommand();
      /** "ERROR" command. */
      static const std::string & errorCommand();
      /** "_ENOMETHOD" command. */
      static const std::string & enomethodCommand();

    public:
      /** Default exception type */
      typedef PluginFrameException Exception;

      /** Default ctor (empty frame) */
      PluginFrame();

      /** Ctor taking the command
       * \throw PluginFrameException If \ref setCommand throws
       */
      PluginFrame( const std::string & command_r );

      /** Ctor taking command and body
       * \throw PluginFrameException If \ref setCommand throws
       */
      PluginFrame( const std::string & command_r, const std::string & body_r );

      /** Ctor taking the command and a HeaderInitializerList
       * \throw PluginFrameException If \ref setCommand throws
       */
      PluginFrame( const std::string & command_r, HeaderInitializerList contents_r );

      /** Ctor taking command, body and a HeaderInitializerList
       * \throw PluginFrameException If \ref setCommand throws
       */
      PluginFrame( const std::string & command_r, const std::string & body_r, HeaderInitializerList contents_r );

      /** Ctor reading frame data from a stream
       * \throw PluginFrameException On error reading from stream
       * \throw PluginFrameException On error parsing the data
       */
      PluginFrame( std::istream & stream_r );

    public:
      /** Whether this is an empty frame. */
      bool empty() const;

      /** Evaluate in a boolean context (not an empty frame) */
      explicit operator bool() const
      { return !empty(); }

    public:
      /** Return the frame command. */
      const std::string & command() const;

      /** Set the frame command
       * \throw PluginFrameException If illegal command string (e.g. multiline)
       */
      void setCommand( const std::string & command_r );

      /** Convenience to identify an ACK command. */
      bool isAckCommand() const
      { return command() == ackCommand(); }

      /** Convenience to identify an ERROR command. */
      bool isErrorCommand() const
      {return command() == errorCommand(); }

      /** Convenience to identify an _ENOMETHOD command. */
      bool isEnomethodCommand() const
      {return command() == enomethodCommand(); }

      /** Return the frame body. */
      const std::string & body() const;

      /** Return a reference to the frame body.
       * This may avoid creating unnecessary copies if you
       * want to manipulate large body data.
       * \code
       *   std::string tmp;
       *   frame.bodyRef().swap( tmp );
       * \endcode
       */
      std::string & bodyRef();

      /** Set the frame body */
      void setBody( const std::string & body_r );

    public:
      /** The header list */
      typedef std::multimap<std::string, std::string> HeaderList;

      /** Header list iterator */
      typedef HeaderList::const_iterator HeaderListIterator;

    private:
      /** Modifyalble header list for internal use only. */
      HeaderList & headerList();

    public:
      /** The header list. */
      const HeaderList & headerList() const;

      /** Whether header list is empty. */
      bool headerEmpty() const
      { return headerList().empty(); }

      /** Return size of the header list. */
      unsigned headerSize() const
      { return headerList().size(); }

      /** Return iterator pointing to the 1st header (or \ref headerEnd) */
      HeaderListIterator headerBegin() const
      { return headerList().begin(); }

      /** Return iterator pointing behind the last header. */
      HeaderListIterator headerEnd() const
      { return headerList().end(); }

      /** Clear the list of headers. */
      void headerClear()
      { headerList().clear(); }


      /** Whether the header list contains at least one entry for \c key_r. */
      bool hasKey( const std::string & key_r ) const
      { return ! keyEmpty( key_r ); }

      /** \overload */
      bool keyEmpty( const std::string & key_r ) const
      { return headerList().find( key_r ) == headerEnd(); }

      /** Return number of header entries for \c key_r. */
      bool keySize( const std::string & key_r ) const
      { return headerList().count( key_r ); }

      /** Return iterator pointing to the 1st header for \c key_r (or \ref keyEnd(key_r)) */
      HeaderListIterator keyBegin( const std::string & key_r ) const
      { return headerList().lower_bound( key_r ); }

      /** Return iterator pointing behind the last header for \c key_r.*/
      HeaderListIterator keyEnd( const std::string & key_r ) const
      { return headerList().upper_bound( key_r ); }


      /** Return header value for \c key_r.
       * \throw PluginFrameException If no header for key_r exists.
       * \throw PluginFrameException If multiple header for key_r exist.
       */
      const std::string & getHeader( const std::string & key_r ) const;

      /** Return header value for \c key_r or \c default_r if it does not exist.
       * \throw PluginFrameException If multiple header for key_r exist.
       */
      const std::string & getHeader( const std::string & key_r, const std::string & default_r ) const;

      /** Not throwing version returing one of the matching header values or \c default_r string. */
      const std::string & getHeaderNT( const std::string & key_r, const std::string & default_r = std::string() ) const;

      /** Set header for \c key_r removing all other occurrences of \c key_r.
       * \throw PluginFrameException If key contains illegal chars (\c NL or \c :)
       * \throw PluginFrameException If value contains illegal chars (\c NL)
       */
      void setHeader( const std::string & key_r, const std::string & value_r = std::string() );

      /** Set a new header list
       * \throw PluginFrameException If key contains illegal chars (\c NL or \c :)
       * \throw PluginFrameException If value contains illegal chars (\c NL)
       */
      void setHeader( HeaderInitializerList contents_r )
      { headerList().clear(); addHeader( contents_r ); }

      /** Add header for \c key_r leaving already existing headers for \c key_r unchanged.
       * \throw PluginFrameException If key contains illegal chars (\c NL or \c :)
       * \throw PluginFrameException If value contains illegal chars (\c NL)
       */
      void addHeader( const std::string & key_r, const std::string & value_r = std::string() );
      /** \overload taking an initializer_list */
      void addHeader( HeaderInitializerList contents_r );

      /** Remove all headers for \c key_r. */
      void clearHeader( const std::string & key_r );

    public:
      /** Write frame to stream
       * \throw PluginFrameException On error writing to stream
       */
      std::ostream & writeTo( std::ostream & stream_r ) const;

      /** \overload Static version. */
      static std::ostream & writeTo( std::ostream & stream_r, const PluginFrame & frame_r )
      { return frame_r.writeTo( stream_r ); }

      /** Read frame from stream
       * \throw PluginFrameException If \ref PluginFrame(std::istream&) throws
       */
      std::istream & readFrom( std::istream & stream_r )
      { *this = PluginFrame( stream_r ); return stream_r; }

      /** \overload Static version. */
      static std::istream & readFrom( std::istream & stream_r, PluginFrame & frame_r )
      { frame_r = PluginFrame( stream_r ); return stream_r; }

    public:
      /** Implementation */
      class Impl;
    private:
      /** Pointer to implementation */
      RWCOW_pointer<Impl> _pimpl;
  };

  /** \relates PluginFrame Stream output for logging */
  std::ostream & operator<<( std::ostream & str, const PluginFrame & obj );

  /** \relates PluginFrame Stream output writing all data for logging (no throw) */
  inline std::ostream & dumpOn( std::ostream & str, const PluginFrame & obj )
  { if ( str ) try { PluginFrame::writeTo( str, obj ); } catch(...){}; return str; }

  /** \relates PluginFrame Construct from stream. */
  inline std::istream & operator>>( std::istream & str, PluginFrame & obj )
  { return PluginFrame::readFrom( str, obj ); }

  /** \relates PluginFrame Comparison based on content. */
  bool operator==( const PluginFrame & lhs, const PluginFrame & rhs );

  /** \relates PluginFrame Comparison based on content. */
  inline bool operator!=( const PluginFrame & lhs, const PluginFrame & rhs )
  { return( ! operator==( lhs, rhs ) ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PLUGINFRAME_H
