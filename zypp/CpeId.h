/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CpeId.h
 */
#ifndef ZYPP_CPEID_H
#define ZYPP_CPEID_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Flags.h"
#include "zypp/base/EnumClass.h"
#include "zypp/base/SetRelationMixin.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class CpeId
  /// \brief Common Platform Enumearation (2.3)
  /// See http://cpe.mitre.org/ for more information on the
  /// Common Platform Enumearation.
  ///////////////////////////////////////////////////////////////////
  class CpeId : public base::SetRelationMixin<CpeId>
  {
  public:
    /** WFN attribute value */
    class Value;

  public:
    /** WFN attributes (use like 'enum class \ref Attribute') */
    struct _AttributeDef {
      enum Enum {
	part,		//< attribute (2.2)
	vendor,		//< attribute (2.2)
	product,	//< attribute (2.2)
	version,	//< attribute (2.2)
	update,		//< attribute (2.2)
	edition,	//< attribute (2.2)
	language,	//< attribute (2.2)
	sw_edition,	//< extended attribute (2.3)
	target_sw,	//< extended attribute (2.3)
	target_hw,	//< extended attribute (2.3)
	other		//< extended attribute (2.3)
      };
      static constexpr unsigned numAttributes = other+1;	///< number of attributes
      static const std::string & asString( Enum val_r );	///< string representantion
    };
    typedef base::EnumClass<_AttributeDef> Attribute;	///< 'enum class Attribute'

  public:
    /** Indicator type for non-trowing ctor. */
    struct NoThrowType {};
    /** Indicator argument for non-trowing ctor. */
    static constexpr NoThrowType noThrow = NoThrowType();

  public:
    /** Default ctor: ANY-Cpeid, all attribute values are ANY */
    CpeId();

    /** Ctor parsing from string representation (empty or URI or FS)
     * \throws std::invalid_argument if string is malformed
     */
    explicit CpeId( const std::string & cpe_r );

    /** Ctor parsing from string representation (empty or URI or FS)
     * \throws std::invalid_argument if string is malformed
     */
    explicit CpeId( const char * cpe_r )
      : CpeId( std::string( cpe_r ? cpe_r : "" ) )
    {}

    /** Ctor parsing from string (empty or URI or FS, non throwing)
     * Creates an empty CpeId if string is malformed.
     */
    CpeId( const std::string & cpe_r, NoThrowType );

    /** Dtor */
    ~CpeId();

  public:
    /**  Evaluate in boolean context: not an ANY-CpeId */
    explicit operator bool() const;

    /** Default string representation [\ref asFS]. */
    std::string asString() const
    { return asFs(); }

    /** String representation as Formated-String (in/out).
     * \code
     * cpe:2.3:a:opensuse:libzypp:14.16.0:beta:*:*:*:*:*:*
     * \endcode
     */
    std::string asFs() const;

    /** String representation as URI (in/out).
     * \code
     * cpe:/a:opensuse:libzypp:14.16.0:beta
     * \endcode
     */
    std::string asUri() const;

    /** String representation as Well-Formed-Name (internal format, out only).
     * \code
     * wfn:[part="a",vendor="opensuse",product="libzypp", version="14\.16\.0",update="beta"]
     * \endcode
     */
    std::string asWfn() const;

  private:
    friend SetCompare base::SetRelationMixin<CpeId>::compare( const CpeId & ) const;
    /** CPE name matching hook for \ref SetRelationMixin */
    SetCompare setRelationMixinCompare( const CpeId & trg ) const;

  public:
    class Impl;                 ///< Implementation class.
  private:
    RWCOW_pointer<Impl> _pimpl; ///< Pointer to implementation.
  };

  SETRELATIONMIXIN_DEFINE_COMPARE_BETWEEN( CpeId, const char * );
  SETRELATIONMIXIN_DEFINE_COMPARE_BETWEEN( CpeId, const std::string & );

  /** \relates CpeId Stream output */
  inline std::ostream & operator<<( std::ostream & str, const CpeId & obj )
  { return str << obj.asString(); }

  /** \relates CpeId::Attribute Stream output */
  inline std::ostream & operator<<( std::ostream & str, const CpeId::Attribute & obj )
  { return str << CpeId::Attribute::asString( obj.asEnum() ); }


  ///////////////////////////////////////////////////////////////////
  /// \class CpeId::Value
  /// \brief WFN attribute value
  ///
  /// Valid WFN string values are not empty, contain alphanumeric chars, underscore,
  /// no whitespace, printable non-alphanumeric characters must be quoted by a
  /// backslash.
  ///
  /// Unescaped asterisk and question mark are special characters and may occur at
  /// the beginning and or end of the string. The asterisk must not be used more than
  /// once in asequense. The question mark may be used more than once in a sequence.
  ///
  /// A single asterisk must not be used as attribute value, nor a single quoted hyphen.
  ///
  /// A single question mark may be used as value.
  ///
  /// \see http://cpe.mitre.org/ for more information on the Common Platform Enumeration.
  ///////////////////////////////////////////////////////////////////
  class CpeId::Value : public base::SetRelationMixin<Value>
  {
  public:
    /** Logical value matching ANY value. */
    static const Value ANY;

    /** Logical value indicating “not applicable/not used" */
    static const Value NA;

  public:
    /** Indicator type for ctor arg in FS format. */
    struct FsFormatType {};
    /** Indicator argument for ctor arg in FS format. */
    static constexpr FsFormatType fsFormat = FsFormatType();

    /** Indicator type for ctor arg in URI format. */
    struct UriFormatType {};
    /** Indicator argument for ctor arg in URI format. */
    static constexpr UriFormatType uriFormat = UriFormatType();

  public:
    /** Default ctor: ANY */
    Value()
    {}

    /** Ctor from string (WFN format; \c "*" represents ANY; \c "" represents NA)
     * \throws std::invalid_argument if string is malformed (\see \ref isValidWfn).
     */
    explicit Value( const std::string & value_r );

    /** Ctor from char* (WFN format; \c nullptr or \c "*" represent ANY; \c "" represents NA)
     * \throws std::invalid_argument if string is malformed (\see \ref isValidWfn).
     */
    explicit Value( const char * value_r )
    : Value( std::string( value_r ? value_r : "*" ) )
    {}

    /** Ctor from string (FS format, \c "*" represents ANY; \c "-" represents NA)
     * \throws std::invalid_argument if string is malformed.
     */
     Value( const std::string & encoded_r, FsFormatType );

    /** Ctor from string (URI format, \c "" represents ANY; \c "-" represents NA)
     * \throws std::invalid_argument if string is malformed.
     */
     Value( const std::string & encoded_r, UriFormatType );

  public:
    /** Classification of \ref Value types mostly for \ref match (use like 'enum class \ref Type') */
    struct _TypeDef {
      enum Enum {
	ANY,
	NA,
	wildcardfree,
	wildcarded,
      };
    };
    typedef base::EnumClass<_TypeDef> Type;	///< 'enum class Type'

    /** Return the \ref Type of this \ref Value. */
    Type type() const
    {
      if ( !_value ) return Type::ANY;
      if ( _value->empty() ) return Type::NA;
      return( isWildcarded() ? Type::wildcarded : Type::wildcardfree );
    }

    /** Whether value is ANY. ) */
    bool isANY() const
    { return !_value; }

    /** Whether value is NA. ) */
    bool isNA() const
    { return _value && _value->empty(); }

    /** Whether it's a logical value (ANY|NA). */
    bool isLogical() const
    { return !_value || _value->empty(); }
    /** \overload for Type */
    bool isLogical( Type type_r ) const
    { return( type_r == Type::ANY || type_r == Type::NA ); }

    /** Whether it's an attribute value string (not logical value). */
    bool isString() const
    { return _value && !_value->empty(); }
    /** \overload for Type */
    bool isString( Type type_r ) const
    { return( type_r == Type::wildcardfree || type_r == Type::wildcarded ); }

    /** An attribute value string without wildcards (<tt>[*?]</tt> at begin and/or end) */
    bool isWildcardfree() const
    { return isString() && ! containsWildcard(); }

    /** An attribute value string with wildcards (<tt>[*?]</tt> at begin and/or end) */
    bool isWildcarded() const
    { return isString() && containsWildcard(); }

  public:
    /** Default string representation [\ref asWfn]. */
    std::string asString() const
    { return asWfn(); }

    /** String representation as in Well-Formed-Name (ANY:"*", NA:"").
     * \code
     * wfn:[part="a",vendor="opensuse",product="libzypp", version="14\.16\.0",update="beta"]
     * \endcode
     */
    std::string asWfn() const;

    /** String representation as in Formated-String (ANY:"*", NA:"-")
     * \code
     * cpe:2.3:a:opensuse:libzypp:14.16.0:beta:*:*:*:*:*:*
     * \endcode
     */
    std::string asFs() const;

    /** String representation as in URI (ANY:"", NA:"-")
     * \code
     * cpe:/a:opensuse:libzypp:14.16.0:beta
     * \endcode
     */
    std::string asUri() const;

  private:
    friend SetCompare base::SetRelationMixin<Value>::compare( const Value & ) const;
    /** CPE name matching hook for \ref SetRelationMixin */
    SetCompare setRelationMixinCompare( const Value & trg ) const;

    /** HAs unquoted <tt>[*?]</tt> at begin and/or end of value.
     * \note \ref isString() must be asserted!
     */
    bool containsWildcard() const;

  private:
    RWCOW_pointer<std::string> _value;
  };

  SETRELATIONMIXIN_DEFINE_COMPARE_BETWEEN( CpeId::Value, const char * );
  SETRELATIONMIXIN_DEFINE_COMPARE_BETWEEN( CpeId::Value, const std::string & );

  /** \relates CpeId::Value Stream output */
  std::ostream & operator<<( std::ostream & str, const CpeId::Value & obj );

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CPEID_H
