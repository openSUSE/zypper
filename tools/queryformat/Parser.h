/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_QUERYFORMAT_PARSER_H
#define ZYPPER_QUERYFORMAT_PARSER_H

#include <exception>
#include <optional>
#include <string_view>

#include "Tokens.h"

///////////////////////////////////////////////////////////////////
namespace zypp::qf
{
  struct ParseException : public std::exception
  {
    ParseException( std::string msg_r )
    : _msg { std::move(msg_r) }
    {}

    const char * what() const throw() override
    { return _msg.c_str(); }

  private:
    std::string _msg;
  };

  /** Translate string into \ref Format.
   * \throws ParseException If parsing did not succeed.
   */
  Format parse( std::string_view qf_r );
  /** \overload */
  inline void parse( std::string_view qf_r, Format & format_r )
  { format_r = parse( qf_r );}


  namespace test {
    /** Parse \a qf_r and compare result against \a expect_r.
     * If \a expect_r is empty or not provided, qf_r itself is expected.
     * If \a expect_r is \c std::nullopt, a \ref ParseException is expected.
     */
    unsigned parsetest( std::string_view qf_r, std::optional<std::string_view> expect_r=std::string_view() );
    /** Run some predefiined tests. */
    unsigned parsetest();
  }
} // namespace zypp::qf
#endif // ZYPPER_QUERYFORMAT_PARSER_H
