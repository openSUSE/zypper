/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/Spirit.h
 *
*/
#ifndef ZYPP_PARSER_SPIRIT_H
#define ZYPP_PARSER_SPIRIT_H

#include <iosfwd>
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor.hpp>
#include <boost/spirit/attribute.hpp>
#include <boost/spirit/dynamic.hpp>
#include <boost/spirit/iterator.hpp>
#include <boost/spirit/utility.hpp>

#include <boost/spirit/phoenix.hpp>
///////////////////////////////////////////////////////////////////
namespace boost
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace spirit
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : file_position_and_offset
    //
    /** A structure to hold positional information.
     * This includes file name and offset, line and column.
     * Suitable to customize position_iterator.
    */
    struct file_position_and_offset
    {
      std::string    file;
      std::streamoff offset;
      unsigned       line;
      unsigned       column;

      file_position_and_offset( const std::string & file_r   = std::string(),
                                std::streamoff      offset_r = 0,
                                unsigned            line_r   = 1,
                                unsigned            column_r = 1 )
      : file  ( file_r )
      , offset( offset_r )
      , line  ( line_r )
      , column( column_r )
      {}

      bool operator==( const file_position_and_offset & rhs ) const
      {
        return( offset == rhs.offset
                && line == rhs.line
                && column == rhs.column
                && file == rhs.file );
      }
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates file_position_and_offset Stream output. */
    std::ostream & operator<<( std::ostream & str, const file_position_and_offset & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : position_policy<file_position_and_offset>
    //
    /** Specialization to handle file_position_and_offset.
     * Track offset, line and column (tab character is not expanded).
    */
    template <>
      class position_policy<file_position_and_offset>
      {
      public:
        void next_line( file_position_and_offset & pos )
        {
          ++pos.offset;
          ++pos.line;
          pos.column = 1;
        }

        //void set_tab_chars( unsigned /*chars*/ ) {}

        void next_char( file_position_and_offset & pos )
        {
          ++pos.offset;
          ++pos.column;
        }

        void tabulation( file_position_and_offset & pos )
        { next_char( pos ); }
      };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace spirit
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace boost
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace spirit
    { /////////////////////////////////////////////////////////////////

      using namespace boost::spirit;

      ////////////////////////////////////////////////////////////////////////////
      //
      //  Parsers
      //
      ////////////////////////////////////////////////////////////////////////////

      struct error_report_parser
      {
        error_report_parser( const char * msg_r )
        : msg( msg_r )
        {}

        typedef nil_t result_t;

        static std::ostream & errstream();

        template <typename ScannerT>
          int operator()( const ScannerT & scan, result_t & /*result*/ ) const
          {
            errstream() << scan.first.get_position() << ' ' << msg << std::endl;
            return -1; // Fail.
          }

        const char * msg;
      };

      typedef functor_parser<error_report_parser> error_report_p;

      ////////////////////////////////////////////////////////////////////////////
      //
      //  Phoenix actors
      //
      ////////////////////////////////////////////////////////////////////////////

      struct push_back_impl
      {
        template <typename Container, typename Item>
          struct result
          {
            typedef void type;
          };

        template <typename Container, typename Item>
          void operator()( Container & container, const Item & item ) const
          {
            container.push_back( item );
          }
      };
      const phoenix::function<push_back_impl> push_back = push_back_impl();

      struct ddump_impl
      {
        template <typename C>
          struct result
          {
            typedef void type;
          };

        template <typename C>
          void operator()( const C & item ) const
          {
            DBG << item << endl;
          }
      };
      const phoenix::function<ddump_impl> ddump = ddump_impl();

      struct mdump_impl
      {
        template <typename C>
          struct result
          {
            typedef void type;
          };

        template <typename C>
          void operator()( const C & item ) const
          {
            MIL << item << endl;
          }
      };
      const phoenix::function<mdump_impl> mdump = mdump_impl();

      /////////////////////////////////////////////////////////////////
    } // namespace spirit
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_SPIRIT_H
