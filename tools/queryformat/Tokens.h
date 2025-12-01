/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_QUERYFORMAT_TOKENS_H
#define ZYPPER_QUERYFORMAT_TOKENS_H

#include <iostream>
#include <optional>
#include <memory>
#include <vector>
#include <string>

#define USE_TAGNAME_IDS 0         // Just code sample how one could use symbol tables
#define STORE_TAGNAME_LOWER 1     // Store parsed tagnames lowercased

///////////////////////////////////////////////////////////////////
namespace std
{
  template <typename Tp>
  std::ostream & operator<<( std::ostream & str, const std::optional<Tp> & obj )
  { if ( obj ) str << *obj; else  str << "(nullopt)"; return str; }
}
///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  // https://rpm-software-management.github.io/rpm/manual/queryformat.html
  // http://ftp.rpm.org/max-rpm/s1-rpm-query-parts.html
  namespace qf
  {
    enum class TokenType
    {
      /** Literal string */
      String,

      /** %-20{TAG:format}
       * A known tag with optional printf like field width and justification:
       * Some TAGs may provide alternative formats representing it (e.g time_t
       * as number or string). A TAG without a value is printed as "(none)".
       * Array TAGs are represented by their 1st entry.
       */
      Tag,

      /** [%{=TAGA} %{TAGB}\n]
       * Parallel iterated arrays. Array TAGs must be equal sized or sized 1 or 0,
       * or use = to lock an array to its 1st entry,
       */
      Array,

      /** %|TAG?{present}:{missing}|
       * Conditional; ternary if tag exists or not
       */
      Conditional,
    };
    inline std::ostream & operator<<( std::ostream & str, const TokenType & obj )
    { return str << "Token[" << static_cast<unsigned>(obj) << "]"; }


    ///////////////////////////////////////////////////////////////////
    /// Base class for Tokens stored in \ref Format.
    struct Token
    {
      Token( TokenType type_r )
      : _type { type_r }
      {}
      virtual ~Token()
      {}
      virtual std::ostream & dumpOn( std::ostream & str ) const
      { return str/* << _type*/; }

      TokenType _type;
    };
    inline std::ostream & operator<<( std::ostream & str, const Token & obj )
    { return obj.dumpOn( str ); }


    ///////////////////////////////////////////////////////////////////
    /// A sequence of Tokens returned from parsing a format string.
    struct Format
    {
      using value_type = Token;
      using ptr_type = std::shared_ptr<value_type>;

      Format()
      {}

      template <typename TToken>
      Format & operator+=( TToken value_r )
      {
        tokens.push_back( ptr_type( new TToken( std::move(value_r) ) ) );
        return *this;
      }

      std::vector<ptr_type> tokens;
    };

    inline std::ostream & operator<<( std::ostream & str, const Format & obj )
    {
      // str << "Format[ " << obj._tokens.size() << "]";
      for ( const auto & el : obj.tokens ) {
        str << *el.get();
      }
      return str;
    }

    ///////////////////////////////////////////////////////////////////
    struct String : public Token
    {
      String()
      : Token { TokenType::String }
      {}
      virtual std::ostream & dumpOn( std::ostream & str ) const override
      { return Token::dumpOn( str ) << value; }

      std::string value;
    };

    ///////////////////////////////////////////////////////////////////
    struct Tag : public Token
    {
      Tag()
      : Token { TokenType::Tag }
      {}
      virtual std::ostream & dumpOn( std::ostream & str ) const override
      { return Token::dumpOn( str ) << "%" << (fieldw?*fieldw:"") << "{" << (noarray?"=":"") << name << (format?":"+*format:"") << "}"; }

#if USE_TAGNAME_IDS
      unsigned name = 0;
#else
      std::string name;
#endif
      std::optional<std::string> fieldw;
      std::optional<std::string> format;
      bool noarray = false;
    };

    ///////////////////////////////////////////////////////////////////
    struct Array : public Token
    {
      Array()
      : Token { TokenType::Array }
      {}
      virtual std::ostream & dumpOn( std::ostream & str ) const override
      { return Token::dumpOn( str ) << "[" << format << "]"; }

      Format format;
    };

    ///////////////////////////////////////////////////////////////////
    struct Conditional : public Token
    {
      Conditional()
      : Token { TokenType::Conditional }
      {}
      virtual std::ostream & dumpOn( std::ostream & str ) const override
      {
        Token::dumpOn( str ) << "%|" << name << "?{" <<  Tformat << "}";
        if ( Fformat ) str <<  ":{" << Fformat << "}";
        return str << "|";
      }

      std::string name;
      Format Tformat;
      std::optional<Format> Fformat;
    };

  } // namespace qf
} // namespace zypp
#endif // ZYPPER_QUERYFORMAT_TOKENS_H
