/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/DrunkenBishop.h
 */
#ifndef ZYPP_BASE_DRUNKENBISHOP_H
#define ZYPP_BASE_DRUNKENBISHOP_H

#include <iosfwd>
#include <vector>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Flags.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace base
  {
    ///////////////////////////////////////////////////////////////////
    /// \class DrunkenBishop
    /// \brief Random art fingerprint visualization
    /// Visualize fingerprint data on a [17x9] (SSH) or [19x11] (GPG) or
    /// custom sized board. The default board size and layout depends on
    /// the data string length (above 32 the GPG board and layout is used).
    ///
    /// The data string should be an even sized HEX string, otherwise
    /// it will be 0-padded.
    ///
    /// All ctor calls may throw \ref std::invalid_argument.
    ///
    /// \code
    ///     [SuSE Package Signing Key <build@suse.de>]
    ///      +------[Title]------+
    ///      |   . . ^           |  fpr FEAB502539D846DB2C0961CA70AF9E8139DB7C82
    ///      |    : : .          |   id 70AF9E8139DB7C82
    ///      |     ^ ^ .         |  cre 1481108255 Wed Dec  7 11:57:35 2016
    ///      |    ^ . l i        |  exp 1607252255 Sun Dec  6 11:57:35 2020
    ///      |   : ^ . f :       |  ttl 992
    ///      |    ? ^ .Sl        |  rpm 39db7c82-5847eb1f
    ///      |   E i ...         |
    ///      |      ^ ..         |
    ///      |       .  .        |
    ///      |        .  .       |
    ///      |         ....      |
    ///      +----[39DB7C82]-----+
    /// \endcode
    ////
    /// Based on https://github.com/atoponce/keyart, the development location
    /// for the Debian signing-party package. We try to use the same charset
    /// and heatmap.
    /// See also http://dirk-loss.de/sshvis/drunken_bishop.pdf.
    ///////////////////////////////////////////////////////////////////
    class DrunkenBishop
    {
      friend std::ostream & operator<<( std::ostream & str, const DrunkenBishop & obj );

    public:
      /** Default ctor: empty board (1x1) */
      DrunkenBishop();

      /** Ctor taking a data string (and optional title) and using a default (SSH/GPG) board. */
      DrunkenBishop( const std::string & data_r, const std::string & title_r = std::string() );

      /** Ctor also taking a desired board height (even value is incremented, width is 2*height-1). */
      DrunkenBishop( const std::string & data_r, const std::string & title_r, unsigned height_r );
      /** Ctor \overload without optional title */
      DrunkenBishop( const std::string & data_r, unsigned height_r )
      : DrunkenBishop( data_r, std::string(), height_r )
      {}

      /** Ctor also taking a desired board height and width (even values are incremented). */
      DrunkenBishop( const std::string & data_r, const std::string & title_r, unsigned height_r, unsigned width_r );
      /** Ctor \overload without optional title  */
      DrunkenBishop( const std::string & data_r, unsigned height_r, unsigned width_r )
      : DrunkenBishop( data_r, std::string(), height_r, width_r )
      {}

      /** Dtor */
      ~DrunkenBishop();

    public:
      /* Rendering options */
      enum OptionBits {
	USE_COLOR	= (1<<0),	///< use colors
      };
      ZYPP_DECLARE_FLAGS(Options,OptionBits);

      /** Render board to steam.*/
      std::ostream & dumpOn( std::ostream & str, Options options_r = Options() ) const
      { return dumpOn( str, std::string(), options_r ); }
      /** \overload taking an indent string prefixing each line. */
      std::ostream & dumpOn( std::ostream & str, const std::string & prefix_r, Options options_r = Options() ) const;

      /** Render board as string.*/
      std::string asString( Options options_r = Options() ) const
      { return asString( std::string(), options_r ); }
      /** \overload taking an indent string prefixing each line. */
      std::string asString( const std::string & prefix_r, Options options_r = Options() ) const;

      /** Render to an array of lines. */
      std::vector<std::string> asLines( Options options_r = Options() ) const
      { return asLines( std::string(), options_r ); }
      /** \overload taking an indent string prefixing each line. */
      std::vector<std::string> asLines( const std::string & prefix_r, Options options_r = Options() ) const;

    public:
      class Impl;              ///< Implementation class.
    private:
      RW_pointer<Impl> _pimpl; ///< Pointer to implementation.
    };

    ZYPP_DECLARE_OPERATORS_FOR_FLAGS(DrunkenBishop::Options);

    /** \relates DrunkenBishop Stream output */
    inline std::ostream & operator<<( std::ostream & str, const DrunkenBishop & obj )
    { return obj.dumpOn( str ); }

  } // namespace base
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_DRUNKENBISHOP_H
