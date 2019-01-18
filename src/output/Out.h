#ifndef OUT_H_
#define OUT_H_

#include <string>
#include <sstream>
#include <boost/format.hpp>
#include <boost/smart_ptr.hpp>

#include <zypp/base/Xml.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/Exception.h>
#include <zypp/base/String.h>
#include <zypp/base/Flags.h>
#include <zypp/base/DefaultIntegral.h>
#include <zypp/base/DtorReset.h>
#include <zypp/Url.h>
#include <zypp/TriBool.h>
#include <zypp/ProgressData.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/base/LogTools.h>

#include "utils/text.h"
#include "utils/colors.h"
#include "utils/prompt.h"
#include "utils/richtext.h"
#include "output/prompt.h"

inline char * asYesNo( bool val_r ) { return val_r ? _("Yes") : _("No"); }
#include "Table.h"
#define OSD ColorStream( std::cout, ColorContext::OSDEBUG )

using namespace zypp;

class Zypper;

///////////////////////////////////////////////////////////////////
namespace text
{
  // translator: usually followed by a ' ' and some explanatory text
  inline ColorString tagNote() { return ColorString( ColorContext::HIGHLIGHT, _("Note:") ); }
  // translator: usually followed by a ' ' and some explanatory text
  inline ColorString tagWarning() { return ColorString( ColorContext::MSG_WARNING, _("Warning:") ); }
  // translator: usually followed by a ' ' and some explanatory text
  inline ColorString tagError() { return ColorString( ColorContext::MSG_ERROR, _("Error:") ); }

  inline const char * qContinue() { return _("Continue?"); }


  /** Simple join of two string types */
  template <class Tltext, class Trtext>
  inline std::string join( const Tltext & ltext, const Trtext & rtext, const char * sep = " " )
  { std::string ret( asString(ltext) ); ret += sep; ret += asString(rtext); return ret; }
}
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace out
{
  static constexpr unsigned termwidthUnlimited = 0u;
  unsigned defaultTermwidth();	// Zypper::instance()->out().termwidth()
} // namespace out
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace out
{
  ///////////////////////////////////////////////////////////////////
  /// \class ListLayout
  /// \brief Basic list layout
  /// \todo fix design made in eile
  ///////////////////////////////////////////////////////////////////
  struct ListLayout
  {
    template <class TFormater> struct Writer;

    ListLayout( bool singleline_r, bool wrapline_r, bool gaped_r, unsigned indent_r )
    : _singleline( singleline_r )
    , _wrapline( wrapline_r )
    , _gaped( gaped_r )
    , _indent( indent_r )
    {}
    bool	_singleline;	///< one list element per line
    bool	_wrapline;	///< fold lines longer than \c _linewidth
    bool	_gaped;		///< add extra NL before element (if singleline)
    unsigned	_indent;	///< ammount of indent
  };

  namespace detail
  {
    template <bool _Singleline, bool _Wrapline, bool _Gaped, unsigned _Indent>
    struct ListLayoutInit : public ListLayout { ListLayoutInit() : ListLayout( _Singleline, _Wrapline, _Gaped, _Indent ) {} };
  }

  typedef detail::ListLayoutInit<true, false,false, 0U>	XmlListLayout;
  typedef detail::ListLayoutInit<true, true, false, 0U>	DefaultListLayout;	///< one element per line, no indent
  typedef detail::ListLayoutInit<true, true, true,  0U>	DefaultGapedListLayout;	///< one element per line, no indent, gaped
  typedef detail::ListLayoutInit<true, true, false, 2U>	IndentedListLayout;	///< one element per line, indented
  typedef detail::ListLayoutInit<true, true, true,  2U>	IndentedGapedListLayout;///< one element per line, indented, gaped
  typedef detail::ListLayoutInit<false,true, false, 2U>	CompressedListLayout;	///< multiple elements per line, indented

  ///////////////////////////////////////////////////////////////////
  /// \class TableLayout
  /// \brief Basic table layout
  ///////////////////////////////////////////////////////////////////
  struct TableLayout
  {
    template <class TFormater> struct Writer;
  };

  typedef TableLayout	DefaultTableLayout;	///< Simple Table

  ///////////////////////////////////////////////////////////////////
  // Either specialize per Type or define a custom Formater:

  /** \relates XmlFormater XML representation of types [no default] */
  template <class Tp>
  std::string asXmlListElement( const Tp & val_r );
  inline std::string asXmlListElement( const std::string & val_r ){ return val_r; }
  inline std::string asXmlListElement( const char * val_r )	{ return val_r; }

  /** \relates ListFormater NORMAL representation of types in lists [no default] */
  template <class Tp>
  std::string asListElement( const Tp & val_r );
  inline std::string asListElement( const std::string & val_r )	{ return val_r; }
  inline std::string asListElement( const char * val_r )	{ return val_r; }

  /** \relates TableFormater NORMAL representation of types as TableHeader [no default] */
  template <class Tp = void>
  TableHeader asTableHeader();

  template <>
  inline TableHeader asTableHeader<void>()
  { return TableHeader(); }

  /** \relates TableFormater NORMAL representation of types as TableRow [no default] */
  template <class Tp>
  TableRow asTableRow( const Tp & val_r );

  ///////////////////////////////////////////////////////////////////
  /// \class XmlFormater
  /// \brief XML representation of types in container [asXmlListElement]
  ///////////////////////////////////////////////////////////////////
  struct XmlFormater
  {
    template <class Tp>
    std::string xmlListElement( const Tp & val_r ) const//< XML representation of element
    { return asXmlListElement( val_r ); }
  };

  ///////////////////////////////////////////////////////////////////
  /// \class ListFormater
  /// \brief Default representation of types in Lists [asListElement]
  ///////////////////////////////////////////////////////////////////
  struct ListFormater : public XmlFormater
  {
    typedef DefaultListLayout	NormalLayout;		//< ListLayout for NORMAL lists

    template <class Tp>
    std::string listElement( const Tp & val_r ) const	//< NORMAL representation of list element
    { return asListElement( val_r ); }
  };

  ///////////////////////////////////////////////////////////////////
  /// \class TableFormater
  /// \brief Special list formater writing a Table [asTableHeader|asTableRow]
  ///////////////////////////////////////////////////////////////////
  struct TableFormater : public XmlFormater
  {
    typedef DefaultTableLayout	NormalLayout;		//< NORMAL layout as Table

    TableHeader header() const				//< TableHeader for TableRow representation
    { return asTableHeader<>(); }

    template <class Tp>					//< Representation as TableRow
    TableRow row( const Tp & val_r ) const
    { return asTableRow( val_r ); }
  };

  ///////////////////////////////////////////////////////////////////
  /// \class XmlFormaterAdaptor
  /// \brief Adaptor mapping xmlListElement->listElement for container XML output
  ///////////////////////////////////////////////////////////////////
   /** Adaptor */
  template <class TFormater>
  struct XmlFormaterAdaptor
  {
    typedef XmlListLayout	NormalLayout;		//< Layout as XML list

    template <class Tp>
    std::string listElement( const Tp & val_r ) const	//< use TFormater::asXmlListElement
    { return _formater.xmlListElement( val_r ); }

    XmlFormaterAdaptor( const TFormater & formater_r )
    : _formater( formater_r )
    {}
  private:
    const TFormater & _formater;
  };

} // namespace out
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace out
{
  ///////////////////////////////////////////////////////////////////
  /// \class ListLayout::Writer
  /// \brief Write out a List according to the layout
  // TODO: wrap singlelines; support for attributed text;
  ///////////////////////////////////////////////////////////////////
  template <class TFormater>
  struct ListLayout::Writer
  {
    NON_COPYABLE( Writer );

    Writer( std::ostream & str_r, const ListLayout & layout_r, const TFormater & formater_r )
    : _str( str_r )
    , _layout( layout_r )
    , _formater( formater_r )
    , _linewidth( defaultTermwidth() )
    , _indent( _layout._indent, ' ' )
    {}

    ~Writer()
    { if ( !_layout._singleline && _cpos ) _str << std::endl; }

    template <class Tp>
    void operator<<( Tp && val_r ) const
    {
      const std::string & element( _formater.listElement( std::forward<Tp>(val_r) ) );

      if ( _layout._singleline )
      {
	if ( _layout._gaped )
	  _str << std::endl;
	_str << _indent << element << std::endl;
      }
      else
      {
	if ( _cpos != 0 && ! fitsOnLine( 1/*' '*/ + element.size() ) )
	  endLine();

	if ( _cpos == 0 )
	{
	  if ( !_indent.empty() )
	    printAndCount( _indent );
	}
	else
	  printAndCount( " " );

	printAndCount( element );
      }
    }

  private:
    bool fitsOnLine( unsigned size_r ) const
    { return( !_layout._wrapline || _linewidth == out::termwidthUnlimited || _cpos + size_r <= _linewidth ); }

    void printAndCount( const std::string & element_r ) const
    { _cpos += element_r.size(); _str << element_r; }

    void endLine() const
    { _str << std::endl; _cpos = 0U; }

  private:
    std::ostream &	_str;
    const ListLayout &	_layout;
    const TFormater &	_formater;
    const unsigned	_linewidth;	///< desired line width
    const std::string	_indent;
    mutable unsigned	_cpos = 0U;
  };

  ///////////////////////////////////////////////////////////////////
  /// \class TableLayout::Writer
  /// \brief Write out a Table according to the layout
  ///////////////////////////////////////////////////////////////////
  template <class TFormater>
  struct TableLayout::Writer
  {
    NON_COPYABLE( Writer );

    Writer( std::ostream & str_r, const TableLayout & layout_r, const TFormater & formater_r )
    : _str( str_r )
    , _layout( layout_r )
    , _formater( formater_r )
    {}

    ~Writer()
    {
      if ( !_t.empty() )
      {
	_t.setHeader( _formater.header() );
	_str << _t;
      }
    }

    template <class Tp>
    void operator<<( Tp && val_r ) const
    { _t.add( _formater.row( std::forward<Tp>(val_r) ) ); }

  private:
    std::ostream &	_str;
    const TableLayout &	_layout;
    const TFormater &	_formater;
    mutable Table	_t;
  };


  /** Write formated container to stream */
  template <class TContainer, class TFormater, class TLayout = typename TFormater::NormalLayout>
  void writeContainer( std::ostream & str_r, const TContainer & container_r, const TFormater & formater_r, const TLayout & layout_r = TLayout() )
  {
    typedef typename TLayout::template Writer<TFormater> Writer;
    Writer writer( str_r, layout_r, formater_r );
    for ( auto && el : container_r )
      writer << el;
  }

  /** Write XML formated container to stream */
  template <class TContainer, class TFormater>
  void xmlWriteContainer( std::ostream & str_r, const TContainer & container_r, const TFormater & formater_r )
  { writeContainer( str_r, container_r, out::XmlFormaterAdaptor<TFormater>(formater_r) ); }

} // namespace out
///////////////////////////////////////////////////////////////////

// Too simple on small terminals as esc-sequences may get truncated.
// A table like writer for attributed strings is desirable.
struct TermLine
{
  enum SplitFlag
  {
    SF_CRUSH	= 1<<0,	//< truncate lhs, then rhs
    SF_SPLIT	= 1<<1,	//< split line across two
    SF_EXPAND	= 1<<2	//< expand short lines iff stdout is a tty
  };
  ZYPP_DECLARE_FLAGS( SplitFlags, SplitFlag );

  TermLine( SplitFlags flags_r, char exp_r ) : flagsHint( flags_r ), expHint( exp_r ) {}
  TermLine( SplitFlags flags_r ) : flagsHint( flags_r ) {}
  TermLine( char exp_r ) : expHint( exp_r ) {}
  TermLine() {}

  SplitFlags flagsHint;				//< flags to use if not passed to \ref get
  zypp::DefaultIntegral<char,' '> expHint;	//< expand char to use if not passed to \ref get
  zypp::DefaultIntegral<int,-1> percentHint;	//< draw progress indicator in expanded space if in [0,100]

  zypp::str::Str lhs;				//< left side
  zypp::str::Str rhs;				//< right side


  /** Return plain line made of lhs + rhs */
  std::string get() const
  { return std::string(lhs) + std::string(rhs); }

  /** Return line optionally formated according to \a width_r and \a flags_r.
   * If \a width_r or \a flags_r is zero a plain line made of lhs + rhs is returned.
   */
  std::string get( unsigned width_r, SplitFlags flags_r, char exp_r ) const;
  /** \overload */
  std::string get( unsigned width_r, SplitFlags flags_r ) const
  { return get( width_r, flags_r, expHint ); }
  /** \overload */
  std::string get( unsigned width_r, char exp_r ) const
  { return get( width_r, flagsHint, exp_r ); }
  /** \overload */
  std::string get( unsigned width_r ) const
  { return get( width_r, flagsHint, expHint ); }
};
ZYPP_DECLARE_OPERATORS_FOR_FLAGS( TermLine::SplitFlags );

/**
 * Base class for producing common (for now) zypper output.
 *
 * This is an abstract class providing interface for writing output like
 * info messages, warnings, error messages, user prompts, progress reports,
 * and download progress reports. See descriptions of the methods for more
 * details.
 *
 * The output is produced using Out derived class instances.
 *
 * <code>
 *
 * // create output object
 * SomePointerType<Out> out;
 * if (options.count("xmlout"))
 *   out = new OutXML();
 * else
 *   out = new OutNormal();
 *
 * out->info("output instance ready to use", Out::HIGH);
 * out->info("Doing foo");
 * try
 * {
 *   foo();
 *   out->prompt(PROMPT_FOO, "Need your action?", "y/n"); // see output/prompt.h
 *   if (action())
 *     out->info("result", Out::QUIET);                // always show the result
 *   else
 *     cout << "special result" << endl; // special output must be done
 *                                       // the usual way for now
 * }
 * catch(const zypp::Exception & e)
 * {
 *   out->error(e, "Problem doing foo", "Do 'bar' to deal with this");
 * }
 *
 * </code>
 */
class Out : private zypp::base::NonCopyable
{
public:
  /** Verbosity levels. */
  typedef enum
  {
    QUIET  = 0,		///< Only important messages (no progress or status, only the result).
    NORMAL = 1,		///< Default output verbosity level. Progress for important tasks, moderate
			///< amount of status messages, operation information, result.
    HIGH   = 2,		///< More detailed description of the operations.
    DEBUG  = 3		///< \todo drop this level in favor of zypper.log?
  } Verbosity;

  /** Known output types implemented by derived classes. */
  enum TypeBit
  {
    TYPE_NORMAL = 0x01<<0,	///< plain text output
    TYPE_XML    = 0x01<<1	///< xml output
  };
  ZYPP_DECLARE_FLAGS(Type,TypeBit);

  static constexpr Type TYPE_NONE	= TypeBit(0x00);
  static constexpr Type TYPE_ALL	= TypeBit(0xff);

protected:
  Out(TypeBit type, Verbosity verbosity = NORMAL)
    : _verbosity(verbosity), _type(type)
  {}

public:
  virtual ~Out();

protected:
  ///////////////////////////////////////////////////////////////////
  /// \class ParentOut
  /// \brief Convenience base class storing the back reference to Out.
  struct ParentOut
  {
    ParentOut( Out & out_r ) : _out( out_r ) {}
    Out & out() { return _out; }
  private:
    Out & _out;
  };

public:
  ///////////////////////////////////////////////////////////////////
  /// \class XmlNode
  /// \brief XML only: RAII writing a XML nodes start/end tag
  /// \code
  /// {
  /// 	XmlNode( "node", { "attr", "val" } ); // <node attr="val">
  /// 	...
  /// }                                       // </node>
  /// \endcode
  struct XmlNode : protected ParentOut
  {
    typedef zypp::xmlout::Node::Attr Attr;

    /** Ctor taking nodename and attribute list. */
    XmlNode( Out & out_r, const std::string & name_r, const std::initializer_list<Attr> & attrs_r = {} )
    : ParentOut( out_r )
    {
      if ( out().typeXML() && ! name_r.empty() )
      { _node.reset( new zypp::xmlout::Node( std::cout, name_r, attrs_r ) ); }
    }

    /** Convenience ctor for one attribute pair */
    XmlNode( Out & out_r, const std::string & name_r, Attr attr_r )
    : XmlNode( out_r, name_r, { attr_r } )
    {}

    /** Move ctor */
    XmlNode( XmlNode && rhs ) : ParentOut( rhs ) { _node.swap( rhs._node ); }

  private:
    scoped_ptr<zypp::xmlout::Node> _node;
  };
  ///////////////////////////////////////////////////////////////////

  /** XML only: Write a leaf node without PCDATA
   * \code
   * <node attr="val"/>
   * \endcode
   */
  void xmlNode( const std::string & name_r, const std::initializer_list<XmlNode::Attr> & attrs_r = {} )
  { if ( typeXML() ) { zypp::xmlout::node( std::cout, name_r, attrs_r ); } }
  /** \overload for one attribute pair */
  void xmlNode( const std::string & name_r, XmlNode::Attr attr_r )
  { xmlNode( name_r, { attr_r } ); }

  ///////////////////////////////////////////////////////////////////
  /// \class TitleNode
  /// \brief XmlNode with optional normal text headline (NL appended)
  struct TitleNode : public XmlNode
  {
    TitleNode( XmlNode && node_r, const std::string & title_r = "" )
    : XmlNode( std::move(node_r) )
    { if ( out().typeNORMAL() && ! title_r.empty() ) std::cout << title_r << std::endl; }
  };

private:
  /** Write container creating a TitleNode with \c size="nnn" attribue and
   * replacing optional \c %1% in \a title_r with size. */
  template <class TContainer, class TFormater>
  void container( const std::string & nodeName_r, const std::string & title_r,
		  const TContainer & container_r, const TFormater & formater_r )
  {
    TitleNode guard( XmlNode( *this, nodeName_r, XmlNode::Attr( "size", str::numstring( container_r.size() ) ) ),
		     str::FormatNAC( title_r ) % container_r.size() );
    switch ( type() )
    {
      case TYPE_NORMAL:
	writeContainer( std::cout, container_r, formater_r );
	break;
      case TYPE_XML:
	xmlWriteContainer( std::cout, container_r, formater_r );
	break;
    }
  }

public:
  /** Write list from container creating a TitleNode with \c size="nnn" attribue and
   * replacing optional \c %1% in \a title_r with size. */
  template <class TContainer, class TFormater = out::ListFormater>
  void list( const std::string & nodeName_r, const std::string & title_r,
	     const TContainer & container_r, const TFormater & formater_r = TFormater() )
  { container( nodeName_r, title_r, container_r, formater_r ); }

  /** Write table from container creating a TitleNode with \c size="nnn" attribue and
   * replacing optional \c %1% in \a title_r with size. */
  template <class TContainer, class TFormater = out::TableFormater>
  void table( const std::string & nodeName_r, const std::string & title_r,
	      const TContainer & container_r, const TFormater & formater_r = TFormater() )
  { container( nodeName_r, title_r, container_r, formater_r ); }

public:
  /** NORMAL: An empty line */
  void gap() { if ( type() == TYPE_NORMAL ) std::cout << std::endl; }

  void printRichText( std::string text, unsigned indent_r = 0U )
  { ::printRichText( std::cout, text, indent_r, termwidth() ); }


  /** Less common Paragraph formats */
  struct ParFormat	// placeholder until we need it
  {};

  /** Paragraph of text, optionally indented, or without leading gap */
  template <class Text>
  void par( size_t indent_r, const Text & text_r, ParFormat format_r = ParFormat() )
  {
    gap();	// if needed make it optional via ParFormat
    str::Str formated;
    mbs_write_wrapped( formated.stream(), asString(text_r), indent_r, defaultFormatWidth( 100 ) );
    info( formated );
  }
  /** \overload  convenience for unindented */
  template <class Text>
  void par( const Text & text_r, ParFormat format_r = ParFormat() )
  { par( 0, text_r, format_r ); }


  /** Paragraph of text preceded by 'tag_r' and a ' ' */
  template <class TText, class Text>
  void taggedPar( size_t indent_r, const TText & tag_r, const Text & text_r, ParFormat format_r = ParFormat() )
  { par( indent_r, text::join( tag_r, text_r ), format_r ); }
  /** \overload convenience for unindented par */
  template <class TText, class Text>
  void taggedPar( const TText & tag_r, const Text & text_r, ParFormat format_r = ParFormat() )
  { taggedPar( 0, tag_r, text_r, format_r ); }


  /** Paragraph tagged with 'Note: ' */
  template <class Text>
  void notePar( size_t indent_r, const Text & text_r, ParFormat format_r = ParFormat() )
  { taggedPar( indent_r, text::tagNote(), text_r, format_r ); }
  /** \overload convenience for unindented par */
  template <class Text>
  void notePar( const Text & text_r, ParFormat format_r = ParFormat() )
  { notePar( 0, text_r, format_r ); }

  /** Paragraph tagged with 'Warning: ' */
  template <class Text>
  void warningPar( size_t indent_r, const Text & text_r, ParFormat format_r = ParFormat() )
  { taggedPar( indent_r, text::tagWarning(), text_r, format_r ); }
  /** \overload convenience for unindented par */
  template <class Text>
  void warningPar( const Text & text_r, ParFormat format_r = ParFormat() )
  { warningPar( 0, text_r, format_r ); }

  /** Paragraph tagged with 'Error: ' */
  template <class Text>
  void errorPar( size_t indent_r, const Text & text_r, ParFormat format_r = ParFormat() )
  { taggedPar( indent_r, text::tagError(), text_r, format_r ); }
  /** \overload convenience for unindented par */
  template <class Text>
  void errorPar( const Text & text_r, ParFormat format_r = ParFormat() )
  { errorPar( 0, text_r, format_r ); }

public:
  /**
   * Show an info message.
   *
   * \param msg       The message to be displayed.
   * \param verbosity Minimal level o verbosity in which the message will be
   *                  shown. Out::QUIET means the message will be always be
   *                  displayed. Out::HIGH means the message will be displayed
   *                  only if the current verbosity level is HIGH (-v) or DEBUG
   *                  (-vv).
   * \param mask      Determines the types of output for which is this message
   *                  intended. By default, the message will be shown in all
   *                  types of output.
   */
  virtual void info(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL) = 0;
  /** \overload taking boost::format */
  void info( const boost::format & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL )
  { info( msg.str(), verbosity, mask ); }
  /** \overload concatenating 2 strings (e.g. translated and untranslated parts) */
  void info( std::string msg, const std::string & msg2, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL )
  { info( (msg+=msg2), verbosity, mask ); }
  /** \overload concatenating 2 strings (e.g. translated and untranslated parts) */
  void info( const boost::format & msg, const std::string & msg2, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL )
  { info( msg.str(), msg2, verbosity, mask ); }

  /** \ref info taking a \ref TermLine */
  virtual void infoLine(const TermLine & msg_r, Verbosity verbosity_r = NORMAL, Type mask_r = TYPE_ALL)
  { info( msg_r.get(), verbosity_r, mask_r ); }

  struct Info : protected ParentOut
  {
    NON_COPYABLE( Info );

    Info( Out & out_r )
      : ParentOut( out_r )
      , _str( new std::ostringstream )
    {}

    Info( Out::Info && rhs )
      : ParentOut( rhs )
      , _str( std::move(rhs._str) )
    {}

    ~Info()
    { out().info( _str->str() ); }

    template<class Tp>
    std::ostream & operator<<( const Tp & val )
    { return (*_str) << val; /*return *this;*/ }

   private:
      std::unique_ptr<std::ostringstream> _str; // work around missing move ctor
  };

  Info info() { return Info( *this ); }

  /**
   * Show a warning.
   *
   * \param msg       The warning message to be displayed.
   * \param verbosity Minimal level o verbosity in which the message will be
   *                  shown. Out::QUIET means the message will be always be
   *                  displayed. Out::HIGH means the message will be displayed
   *                  only if the current verbosity level is HIGH (-v) or DEBUG
   *                  (-vv).
   * \param mask      Determines the types of output for which is this message
   *                  intended. By default, the message will be shown in all
   *                  types of output.
   */
  virtual void warning(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL) = 0;
  void warning( const boost::format & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL )
  { warning( msg.str(), verbosity, mask ); }

  /** Convenience class for error reporting. */
  class Error;

  /**
   * Show an error message and an optional hint.
   *
   * An error message should be shown regardless of the verbosity level.
   *
   * \param problem_desc Problem description (what happend)
   * \param hint         Hint for the user (what to do, or explanation)
   */
  virtual void error(const std::string & problem_desc, const std::string & hint = "") = 0;
  void error( const boost::format & problem_desc, const std::string & hint = "")
  { error( problem_desc.str(), hint ); }

  /**
   * Prints the problem description caused by an exception, its cause and,
   * optionaly, a hint for the user.
   *
   * \param e Exception which caused the problem.
   * \param Problem description for the user.
   * \param Hint for the user how to cope with the problem.
   */
  virtual void error(const zypp::Exception & e,
                     const std::string & problem_desc,
                     const std::string & hint = "") = 0;

  //! \todo provide an error() method with a/r/i prompt, more read_action_ari here

  /** \name Progress of an operation. */
  //@{

  /** Convenience class for progress output. */
  class ProgressBar;

  /** Convenience class for download progress output. */
  class DownloadProgress;

  /**
   * Start of an operation with reported progress.
   *
   * \param id      Identifier. Any string used to match multiple overlapping
   *                progress reports (doesn't happen now,
   *                but probably will in the future).
   * \param label   Progress description.
   * \param is_tick <tt>false</tt> for known progress percentage, <tt>true</tt>
   *                for 'still alive' notifications
   */
  virtual void progressStart(const std::string & id,
                             const std::string & label,
                             bool is_tick = false) = 0;

  /**
   * Progress report for an on-going operation.
   *
   * \param id      Identifier. Any string used to match multiple overlapping
   *                progress reports.
   * \param label   Progress description.
   * \param value   Percentage value or <tt>-1</tt> if unknown ('still alive'
   *                notification)
   */
  virtual void progress(const std::string & id,
                        const std::string & label,
                        int value = -1) = 0;

  /**
   * End of an operation with reported progress.
   *
   * \param id      Identifier. Any string used to match multiple overlapping
   *                progress reports.
   * \param label   Progress description.
   * \param error   <tt>false</tt> if the operation finished with success,
   *                <tt>true</tt> otherwise.
   */
  virtual void progressEnd(const std::string & id,
                           const std::string & label,
                           bool error = false) = 0; // might be a string with error message instead
  //@}

  /** \name Download progress with download rate */
  //@{
  /**
   * Reoprt start of a download.
   *
   * \param uri   Uri of the file to download.
   */
  virtual void dwnldProgressStart(const zypp::Url & uri) = 0;

  /**
   * Reports download progress.
   *
   * \param uri   Uri of the file being downloaded.
   * \param value Value of the progress in percents. -1 if unknown.
   * \param rate  Current download rate in B/s. -1 if unknown.
   */
  virtual void dwnldProgress(const zypp::Url & uri,
                             int value = -1,
                             long rate = -1) = 0;
  /**
   * Reports end of a download.
   *
   * \param uri   Uri of the file to download.
   * \param rate  Average download rate at the end. -1 if unknown.
   * \param error Error flag - did the download finish with error?
   */
  virtual void dwnldProgressEnd(const zypp::Url & uri,
                                long rate = -1,
                                bool error = false) = 0;
  //@}

  /**
   * Print out a search result.
   *
   * Default implementation prints \a table_r on \c stdout.
   *
   * \param table_r Table containing the search result.
   *
   * \todo Using a more generic format than a Table is desired.
   */
  virtual void searchResult( const Table & table_r );

  /**
   * Prompt the user for a decision.
   *
   * \param id           Unique prompt identifier for use by machines.
   * \param prompt       Prompt text.
   * \param options      A PromptOptions object
   * \param startdesc    Initial detailed description of the prompt to be
   *                     prepended to the \a prompt text. Should be used
   *                     only whe prompting for the first time and left empty
   *                     when retrying after an invalid answer has been given.
   * \see prompt.h
   * \see ../zypper-prompt.h
   */
  virtual void prompt(PromptId id,
                      const std::string & prompt,
                      const PromptOptions & poptions,
                      const std::string & startdesc = "") = 0;

  /**
   * Print help for prompt, if available.
   * This method should be called after '?' prompt option has been entered.
   */
  virtual void promptHelp(const PromptOptions & poptions) = 0;

public:
  /** Get current verbosity. */
  Verbosity verbosity() const { return _verbosity; }

  /** Set current verbosity. */
  void setVerbosity(Verbosity verbosity) { _verbosity = verbosity; }

  /** Convenience macro for exception safe scoped verbosity change
   * \code
   *   {
   *     // shut up zypper
   *     SCOPED_VERBOSITY( Zypper::instance()->out(), Out::QUIET );
   *     // expands to:
   *     // const auto & raii __attribute__ ((__unused__))( Zypper::instance()->out().scopedVerbosity( Out::QUIET ) );
   *     ...
   *     // leaving the block restores previous verbosity
   *   }
   * \endcode
   */
#define SCOPED_VERBOSITY( OUT, LEVEL ) const auto & raii __attribute__ ((__unused__))( (OUT).scopedVerbosity( LEVEL ))

  /** Return RAII class for exception safe scoped verbosity change. */
  DtorReset scopedVerbosity( Verbosity verbosity_r )
  {
    std::swap( _verbosity, verbosity_r );
    return DtorReset( _verbosity, verbosity_r );
  }


public:
  /** Return the type of the instance. */
  TypeBit type() const { return _type; }

  /** Test for a specific type */
  bool type( TypeBit type_r ) const { return type() == type_r; }
  /** \overload test for TYPE_NORMAL */
  bool typeNORMAL() const { return type( TYPE_NORMAL ); }
  /** \overload test for TPE_XML */
  bool typeXML() const { return type( TYPE_XML ); }

  /** Terminal width or 150 if unlimited.
   * If a \a desired_r value is given, return the
   * closest width that fits the terminal.
   */
  unsigned defaultFormatWidth( unsigned desired_r = 0 ) const
  {
    unsigned ret = termwidth();
    if ( ret == out::termwidthUnlimited )
      ret = desired_r ? desired_r : 150U;
    else if ( desired_r < ret )
      ret = desired_r;
    return ret;
  }

  /** Width for formated output [0==unlimited]. */
  virtual unsigned termwidth() const { return out::termwidthUnlimited; }

protected:

  /**
   * Determine whether the output is intended for the particular type.
   */
  virtual bool mine(Type type) = 0;

  /**
   * Determine whether to show progress.
   *
   * \return <tt>true</tt> if the progress should be filtered out,
   *         <tt>false</tt> if it should be shown.
   */
  virtual bool progressFilter();

  /**
   * Return a zypp::Exception as a string suitable for output.
   */
  virtual std::string zyppExceptionReport(const zypp::Exception & e);

private:
  Verbosity _verbosity;
  const TypeBit _type;
};

ZYPP_DECLARE_OPERATORS_FOR_FLAGS(Out::Type);

///////////////////////////////////////////////////////////////////
/// \class Out::ProgressBar
/// \brief Convenience class for progress output.
///
/// Progress start and end messages are provided upon object
/// construction and deletion. Progress data are sent through a
/// zypp::ProgressData object accessible via \ref operator->.
///
/// \code
///    {
///      Out::ProgressBar report( _zypper.out(), "Prepare action" );
///      for ( unsigned i = 0; i < 10; ++ i )
///      {
///        report->tick();	// turn wheel
///        sleep(1);
///      }
///      report->range( 10 );	// switch to percent mode [0,10]
///      report.print( "Running action" );
///      for ( unsigned i = 0; i < 10; ++ i )
///      {
///        report->
///        report->set( i );	// send 0%, 10%, ...
///        sleep(1);
///      }
///      // report.error( "Action failed" );
///    }
/// \endcode
///
/// If non zero values for \a current_r or \a total_r are passed to
/// the ctor, the label is prefixed by either "(#C)" or "(#C/#T)"
///
/// \todo ProgressData provides NumericId which might be used as
/// id for_out.progress*().
///////////////////////////////////////////////////////////////////
class Out::ProgressBar : private zypp::base::NonCopyable
{
public:
  /** Indicator type for ctor not drawing an initial start bar. */
  struct NoStartBar {};
  /** Indicator argument for ctor not drawing an initial start bar.*/
  static constexpr NoStartBar noStartBar = NoStartBar();

public:
  /** Ctor not displaying an initial progress bar.
   * If non zero values for \a current_r or \a total_r are passed,
   * the label is prefixed by either "(#C)" or "(#C/#T)"
   */
  ProgressBar( Out & out_r, NoStartBar, const std::string & progressId_r, const std::string & label_r, unsigned current_r = 0, unsigned total_r = 0 )
    : _out( out_r )
    , _error( indeterminate )
    , _progressId( progressId_r )
  {
    if ( total_r )
      _labelPrefix = zypp::str::form( "(%*u/%u) ", numDigits( total_r ), current_r, total_r );
    else if ( current_r )
      _labelPrefix = zypp::str::form( "(%u) ", current_r );
    _progress.name( label_r );
    _progress.sendTo( Print( *this ) );
  }
  ProgressBar( Out & out_r, NoStartBar, const std::string & progressId_r, const boost::format & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, noStartBar, progressId_r, label_r.str(), current_r, total_r )
  {}

  ProgressBar( Out & out_r,NoStartBar,  const std::string & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, noStartBar, "", label_r, current_r, total_r )
  {}

  ProgressBar( Out & out_r, NoStartBar, const boost::format & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, noStartBar, "", label_r.str(), current_r, total_r )
  {}


  /** Ctor displays initial progress bar.
   * If non zero values for \a current_r or \a total_r are passed,
   * the label is prefixed by either "(#C)" or "(#C/#T)"
   */
  ProgressBar( Out & out_r, const std::string & progressId_r, const std::string & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, noStartBar, progressId_r, label_r, current_r, total_r )
  {
    // print the initial progress bar
    _out.progressStart( _progressId, outLabel( _progress.name() ) );
  }

  ProgressBar( Out & out_r, const std::string & progressId_r, const boost::format & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, progressId_r, label_r.str(), current_r, total_r )
  {}

  ProgressBar( Out & out_r, const std::string & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, "", label_r, current_r, total_r )
  {}

  ProgressBar( Out & out_r, const boost::format & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, "", label_r.str(), current_r, total_r )
  {}

  /** Dtor displays final progress bar.
    * Unless \ref error has explicitly been set, an error is indicated if
    * a \ref ProgressData range has been set, but 100% were not reached.
    */
  ~ProgressBar()
  {
    _progress.noSend();	// suppress ~ProgressData final report
    if ( indeterminate( _error ) )
      _error = ( _progress.reportValue() != 100 && _progress.reportPercent() );
    _out.progressEnd( _progressId, outLabel( _progress.name() ), _error );
  }

  /** Immediately print the progress bar not waiting for a new trigger. */
  void print()
  { _out.progress( _progressId, outLabel( _progress.name() ), _progress.reportValue() ); }

  /** \overload also change the progress bar label. */
  void print( const std::string & label_r )
  { _progress.name( label_r ); print(); }

  /** Explicitly indicate the error condition for the final progress bar. */
  void error( tribool error_r = true )
  { _error = error_r; }

  /** \overload to disambiguate. */
  void error( bool error_r )
  { _error = error_r; }

  /** \overload also change the progress bar label. */
  void error( const std::string & label_r )
  { _progress.name( label_r ); error( true ); }

  /** \overload also change the progress bar label and disambiguate. */
  void error( const char * label_r )
  { _progress.name( label_r ); error( true ); }

  /** \overload also change the progress bar label. */
  void error( tribool error_r, const std::string & label_r )
  { _progress.name( label_r ); error( error_r ); }

public:
  /** \name Access the embedded ProgressData object */
  //@{
  zypp::ProgressData * operator->()
  { return &_progress; }

  const zypp::ProgressData * operator->() const
  { return &_progress; }

  zypp::ProgressData & operator*()
  { return _progress; }

  const zypp::ProgressData & operator*() const
  { return _progress; }
  //@}

private:
  /** ProgressData::ReceiverFnc printing to a ProgressBar.
    *
    * \note This could also be used to let an external \ref ProgressData object
    * trigger a \ref ProgressBar. \ref ProgressBar::label and \ref ProgressBar::print
    * however use the embedded ProgressData object (esp. it's label). So don't mix this.
    */
  struct Print
  {
    Print( ProgressBar & bar_r ) : _bar( &bar_r ) {}
    bool operator()( const ProgressData & progress_r )
    { _bar->_out.progress( _bar->_progressId, _bar->outLabel( progress_r.name() ), progress_r.reportValue() ); return true; }
  private:
    ProgressBar * _bar;
  };

  std::string outLabel( const std::string & msg_r ) const
  { return _labelPrefix.empty() ? msg_r : _labelPrefix + msg_r; }

  int numDigits( unsigned num_r ) const
  { int ret = 1; while ( num_r /= 10 ) ++ret; return ret; }

private:
  Out & _out;
  tribool _error;
  ProgressData _progress;
  std::string _progressId;
  std::string _labelPrefix;
};
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
/// \class Out::DownloadProgress
/// \brief Listen on media::DownloadProgressReport to feed a ProgressBar.
///
/// Connect to media::DownloadProgressReport to feed a ProgressBar, but forward
/// callbacks to any original receiver.
///////////////////////////////////////////////////////////////////
struct Out::DownloadProgress : public callback::ReceiveReport<media::DownloadProgressReport>
{
  DownloadProgress( Out::ProgressBar & progressbar_r )
  : _progressbar( &progressbar_r )
  , _oldReceiver( Distributor::instance().getReceiver() )
  {
    connect();
  }

  ~DownloadProgress()
  {
    if ( _oldReceiver )
      Distributor::instance().setReceiver( *_oldReceiver );
    else
      Distributor::instance().noReceiver();
  }

  virtual void start( const Url & file, Pathname localfile )
  {
    (*_progressbar)->range( 100 );	// we'll receive %

    if ( _oldReceiver )
      _oldReceiver->start( file, localfile );
  }

  virtual bool progress( int value, const Url & file, double dbps_avg = -1, double dbps_current = -1 )
  {
    (*_progressbar)->set( value );

    if ( _oldReceiver )
      return _oldReceiver->progress( value, file, dbps_avg, dbps_current );
    return true;
  }

  virtual Action problem( const Url & file, Error error, const std::string & description )
  {
    ERR << description << endl;

    if ( _oldReceiver )
      return _oldReceiver->problem( file, error, description );
    return Receiver::problem( file, error, description );
  }

  virtual void finish( const Url & file, Error error, const std::string & reason )
  {
    if ( error == NO_ERROR )
      (*_progressbar)->toMax();
    else
    {
      ERR << reason << std::endl;
      _progressbar->error();
    }

    if ( _oldReceiver )
      _oldReceiver->finish( file, error, reason );
  }

private:
  Out::ProgressBar * _progressbar;
  Receiver * _oldReceiver;
};
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
/// \class Out::Error
/// \brief Convenience class Error reporting.
///
/// Called action methods may \c throw this as exception. The calling function
/// should catch and process it (e.g. by calling \ref report).
///
/// This allows e.g. active \ref Out::ProgressBar objects to close properly
/// before the error message is displayed.
///
/// \code
///     try {
///       Out::ProgressBar report( zypper_r.out(), _("Scanning download directory") );
///       report->range( todolist.size() );
///       // now report will indicate an error id closed before reaching 100%
///       ....
///       if ( some error )
///         throw( Out::Error( ZYPPER_EXIT_ERR_BUG,
///                           _("Failed to read download directory"),
///                           Errno().asString() ) );
///
///     }
///     catch ( const SourceDownloadImpl::Error & error_r )
///     {
///       // Default way of processing a caught Error exception:
///       // - Write error message and optional hint to screen.
///       // - Set the ZYPPER_EXIT_ code if necessary.
///       // - Return the current ZYPPER_EXIT_ code.
///       return error_r.report( zypper_r );
///     }
/// \endcode
///////////////////////////////////////////////////////////////////
struct Out::Error
{
  Error()
  : _exitcode( ZYPPER_EXIT_OK ) {}
  Error( int exitcode_r )
  : _exitcode( exitcode_r ) {}

  // basic: code msg hint
  Error( int exitcode_r, std::string msg_r, std::string hint_r = std::string() )
  : _exitcode( exitcode_r ), _msg( std::move(msg_r) ), _hint( std::move(hint_r) ) {}
  Error( int exitcode_r, const boost::format & msg_r, std::string hint_r = std::string() )
  : _exitcode( exitcode_r ), _msg( boost::str( msg_r ) ), _hint( std::move(hint_r) ) {}
  Error( int exitcode_r, std::string msg_r, const boost::format & hint_r )
  : _exitcode( exitcode_r ), _msg( std::move(msg_r) ), _hint( boost::str( hint_r ) ) {}
  Error( int exitcode_r, const boost::format & msg_r, const boost::format & hint_r )
  : _exitcode( exitcode_r ), _msg( boost::str( msg_r ) ), _hint( boost::str( hint_r ) ) {}

  // code exception hint
  Error( int exitcode_r, const zypp::Exception & ex_r, std::string hint_r = std::string() )
  : _exitcode( exitcode_r ), _msg( combine( ex_r ) ), _hint( std::move(hint_r) ) {}
  Error( int exitcode_r, const zypp::Exception & ex_r, const boost::format & hint_r )
  : _exitcode( exitcode_r ), _msg( combine( ex_r ) ), _hint( boost::str( hint_r ) ) {}

  // code (msg exception) hint
  Error( int exitcode_r, std::string msg_r, const zypp::Exception & ex_r, std::string hint_r = std::string() )
  : _exitcode( exitcode_r ), _msg( combine( std::move(msg_r), ex_r ) ), _hint( std::move(hint_r) ) {}
  Error( int exitcode_r, const boost::format & msg_r, const zypp::Exception & ex_r, std::string hint_r = std::string() )
  : _exitcode( exitcode_r ), _msg( combine( boost::str( msg_r ), ex_r ) ), _hint( std::move(hint_r) ) {}
  Error( int exitcode_r, std::string msg_r, const zypp::Exception & ex_r, const boost::format & hint_r )
  : _exitcode( exitcode_r ), _msg( combine( std::move(msg_r), ex_r ) ), _hint( boost::str( hint_r ) ) {}
  Error( int exitcode_r, const boost::format & msg_r, const zypp::Exception & ex_r, const boost::format & hint_r )
  : _exitcode( exitcode_r ), _msg( combine( boost::str( msg_r ), ex_r ) ), _hint( boost::str( hint_r ) ) {}


  // as above but without code	ZYPPER_EXIT_OK
  Error( std::string msg_r, std::string hint_r = std::string() )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( std::move(msg_r) ), _hint( std::move(hint_r) ) {}
  Error( const boost::format & msg_r, std::string hint_r = std::string() )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( boost::str( msg_r ) ), _hint( std::move(hint_r) ) {}
  Error( std::string msg_r, const boost::format & hint_r )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( std::move(msg_r) ), _hint( boost::str( hint_r ) ) {}
  Error( const boost::format & msg_r, const boost::format & hint_r )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( boost::str( msg_r ) ), _hint( boost::str( hint_r ) ) {}

  Error( const zypp::Exception & ex_r, std::string hint_r = std::string() )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( combine( ex_r ) ), _hint( std::move(hint_r) ) {}
  Error( const zypp::Exception & ex_r, const boost::format & hint_r )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( combine( ex_r ) ), _hint( boost::str( hint_r ) ) {}

  Error( std::string msg_r, const zypp::Exception & ex_r, std::string hint_r = std::string() )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( combine( std::move(msg_r), ex_r ) ), _hint( std::move(hint_r) ) {}
  Error( const boost::format & msg_r, const zypp::Exception & ex_r, std::string hint_r = std::string() )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( combine( boost::str( msg_r ), ex_r ) ), _hint( std::move(hint_r) ) {}
  Error( std::string msg_r, const zypp::Exception & ex_r, const boost::format & hint_r )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( combine( std::move(msg_r), ex_r ) ), _hint( boost::str( hint_r ) ) {}
  Error( const boost::format & msg_r, const zypp::Exception & ex_r, const boost::format & hint_r )
  : _exitcode( ZYPPER_EXIT_OK ), _msg( combine( boost::str( msg_r ), ex_r ) ), _hint( boost::str( hint_r ) ) {}



  /** Default way of processing a caught \ref Error exception.
   * \li Write error message and optional hint to screen.
   * \li Set the ZYPPER_EXIT_ code if necessary.
   * \returns the zypper exitcode.
   */
  int report( Zypper & zypper_r ) const;

  int _exitcode;	//< ZYPPER_EXIT_OK indicates exitcode is already set.
  std::string _msg;
  std::string _hint;

private:
  static std::string combine( std::string && msg_r, const zypp::Exception & ex_r );
  static std::string combine( const zypp::Exception & ex_r );
};
///////////////////////////////////////////////////////////////////

#endif /*OUT_H_*/
