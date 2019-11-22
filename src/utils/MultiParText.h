/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef UTILS_MULTIPARTEXT_H
#define UTILS_MULTIPARTEXT_H

#include <string>
#include <initializer_list>

#include <zypp/base/String.h>	// zypp::asString

///////////////////////////////////////////////////////////////////
/// \brief Helper to build multi paragraph description texts
///
/// We prefer translated description paragraphs to contain no
/// format information (parindent,parsep,NL). The class joins
/// multiple paragraphs passed to the ctor so they are rendered
/// correctly.
///
struct MultiParText
{
  MultiParText() = default;

  template <typename Tstr>
  MultiParText( Tstr str_r )
  : _text { zypp::asString(str_r) }
  {}

  template <typename... Targs >
  MultiParText( Targs&&... args_r )
  { int __attribute__((unused)) dummy[] = { 0, ( (void)appender( _text, std::forward<Targs>(args_r) ), 0 )... }; }

  const std::string & str() const& { return _text; }
  std::string && str() && { return std::move(_text); }

  const std::string & asString() const& { return _text; }
  std::string && asString() && { return std::move(_text); }

  operator const std::string &() & { return _text; }	// Intentionally not const& qualified. GCC-9 sees an ambiguity in `std::string( MultiParText() );`
  operator std::string &&() && { return std::move(_text); }

private:
  template <typename Tstr>
  static void appender( std::string & result_r, Tstr str_r )
  { result_r += zypp::asString(str_r); }

  std::string _text;
};

/** \relates MultiParText */
inline std::ostream & operator<<( std::ostream & str, const MultiParText & obj )
{ return str << obj.str(); }

#endif // UTILS_MULTIPARTEXT_H
