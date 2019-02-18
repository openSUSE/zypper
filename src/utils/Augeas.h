/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTIL_AUGEAS_H_
#define ZYPPER_UTIL_AUGEAS_H_

#include <iosfwd>
#include <string>

#include <zypp/base/PtrTypes.h>
#include <zypp/Pathname.h>

struct augeas;

///////////////////////////////////////////////////////////////////
/// \class Augeas
/// \brief Zypper's wrapper around Augeas.
///////////////////////////////////////////////////////////////////
class Augeas
{
  NON_COPYABLE_BUT_MOVE( Augeas );
  friend std::ostream & operator<<( std::ostream & str_r, const Augeas & obj_r );

public:
  /** Ctor opt. taking a custom config file (otherwise the default cfg files are used). */
  Augeas( zypp::Pathname customcfg_r = zypp::Pathname() );

  ~Augeas();

public:
  /** Returns the value for \a option_r ("SECTION/VARIABLE") or an empty string. */
  std::string getOption( const std::string & option_r ) const;

public:
  class Impl;
private:
  zypp::RW_pointer<Impl,zypp::rw_pointer::Scoped<Impl>> _pimpl; ///< Pointer to implementation
};

#endif /* ZYPPER_UTIL_AUGEAS_H_ */
