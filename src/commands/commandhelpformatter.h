#ifndef ZYPPER_COMMANDS_COMMANDHELPFORMATTER_INCLUDED
#define ZYPPER_COMMANDS_COMMANDHELPFORMATTER_INCLUDED

#include "Zypper.h"

///////////////////////////////////////////////////////////////////
/// \class CommandHelpFormater
/// \brief Class for command help formating
///////////////////////////////////////////////////////////////////
struct CommandHelpFormater
{
  CommandHelpFormater()
    : _mww( _str, Zypper::instance().out().defaultFormatWidth( 100 ) )
  {}

  /** Allow using the underlying steam directly. */
  template<class Tp_>
  CommandHelpFormater & operator<<( Tp_ && val )
  { _str << std::forward<Tp_>(val); return *this; }

  /** Conversion to std::string */
  operator std::string() const { return _str.str(); }

  /** An empty line */
  CommandHelpFormater & gap()
  { _mww.gotoNextPar(); return *this; }

  /** Synopsis
   * \code
   * "<singleline text_r>"
   * \endcode
   */
  CommandHelpFormater & synopsis( boost::string_ref text_r )
  { _mww.writePar( text_r ); return *this; }
  /** \overload const char * text */
  CommandHelpFormater & synopsis( const char * text_r )
  { return synopsis( boost::string_ref(text_r) ); }
  /** \overload std::string text */
  CommandHelpFormater & synopsis( const std::string & text_r )
  { return synopsis( boost::string_ref(text_r) ); }
  /** \overload str::Format text */
  CommandHelpFormater & synopsis( const str::Format & text_r )
  { return synopsis( boost::string_ref(text_r.str()) ); }


  /** Description block with leading gap
   * \code
   *
   * "<multiline text_r>"
   * \endcode
   */
  CommandHelpFormater & description( boost::string_ref text_r )
  { _mww.gotoNextPar(); _mww.writePar( text_r ); return *this; }
  /** \overload const char * text */
  CommandHelpFormater & description( const char * text_r )
  { return description( boost::string_ref(text_r) ); }
  /** \overload std::string text */
  CommandHelpFormater & description( const std::string & text_r )
  { return description( boost::string_ref(text_r) ); }
  /** \overload str::Format text */
  CommandHelpFormater & description( const str::Format & text_r )
  { return description( boost::string_ref(text_r.str()) ); }

  /** Option section title
   * \code
   * ""
   * "  <text_r:>"
   * ""
   * \endcode
   */
  CommandHelpFormater & optionSection( boost::string_ref text_r )
  { _mww.gotoNextPar(); _mww.writePar( text_r, 2 ); _mww.gotoNextPar(); return *this; }

  CommandHelpFormater & optionSectionCommandOptions()
  { return optionSection(_("Command options:") ); }

  CommandHelpFormater & optionSectionSolverOptions()
  { return optionSection(_("Solver options:") ); }

  CommandHelpFormater & optionSectionExpertOptions()
  { return optionSection(_("Expert options:") ); }

  CommandHelpFormater & noOptionSection()
  { return optionSection(_("This command has no additional options.") ); }

  CommandHelpFormater & legacyOptionSection()
  { return optionSection(_("Legacy options:") ); }

  CommandHelpFormater & legacyOption( boost::string_ref old_r, boost::string_ref new_r )
  { // translator: '-r             The same as -f.
    return option( old_r, str::Format(_("The same as %1%.")) % new_r ); }


  /** Option definition
   * \code
   * "123456789012345678901234567890123456789
   * "-o, --long-name             <text_r> starts on 29 maybe on next line"
   * "                            if long-name is too long.
   * \endcode
   */
  CommandHelpFormater & option( boost::string_ref option_r, boost::string_ref text_r )
  { _mww.writeDefinition( option_r , text_r, (option_r.starts_with( "--" )?4:0), 28 ); return *this; }
  /** \overload const char * text */
  CommandHelpFormater & option( boost::string_ref option_r, const char * text_r )
  { return option( option_r, boost::string_ref(text_r) ); }
  /** \overload std::string text */
  CommandHelpFormater & option( boost::string_ref option_r, const std::string & text_r )
  { return option( option_r, boost::string_ref(text_r) ); }
  /** \overload str::Format text */
  CommandHelpFormater & option( boost::string_ref option_r, const str::Format & text_r )
  { return option( option_r, boost::string_ref(text_r.str()) ); }
  /** \overload "option\ntext_r" */
  CommandHelpFormater & option( boost::string_ref allinone_r )
  {
    std::string::size_type sep = allinone_r.find( '\n' );
    if ( sep != std::string::npos )
      _mww.writeDefinition( allinone_r.substr( 0, sep ), allinone_r.substr( sep+1 ), (allinone_r.starts_with( "--" )?4:0), 28 );
    else
      _mww.writeDefinition( allinone_r , "", (allinone_r.starts_with( "--" )?4:0), 28 );
    return *this;
  }

  /** \todo eliminate legacy indentation */
  CommandHelpFormater & option26( boost::string_ref option_r, boost::string_ref text_r )
  { _mww.writeDefinition( option_r , text_r, (option_r.starts_with( "--" )?4:0), 26 ); return *this; }

private:
  std::ostringstream   _str;
  mbs::MbsWriteWrapped _mww;
};

#endif
