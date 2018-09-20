#ifndef ZYPPER_COMMANDS_COMMANDHELPFORMATTER_INCLUDED
#define ZYPPER_COMMANDS_COMMANDHELPFORMATTER_INCLUDED

#include "Zypper.h"

///////////////////////////////////////////////////////////////////
/// \class CommandHelpFormater
/// \brief Class for command help formating
///
/// \note Changing the output format \c bash-completion.sh must be
/// checked. Some format elements are used to parse the options
/// out of the help texts:
///
/// Within the command/global help e.g. only options are recognized
/// which occur AFTER a line matching <tt>/[Oo]ptions:$/</tt>
/// and BEFORE a line matching <tt>/[Cc]ommands:$/</tt>.
///
/// Unlike the command help, the global help expects the longopts
/// first,
///
/// Within the global help commands are recognized AFTER a a line
/// matching <tt>/[Cc]ommands:$/</tt>.
///
/// Command names are indented by 6 and NO OTHER line may use this
//  indent.
///
/// (But better check the bash-completion code if you plan changes...)
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

  /** Description block stating 'This is an alias for...' */
  CommandHelpFormater & descriptionAliasCmd( const char * command_r )
  { // translator: %s is an other command: "This is an alias for 'zypper info -t patch'."
    return description( str::Format(_("This is an alias for '%s'.")) % command_r ); }

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


  ///////////////////////////////////////////////////////////////////
  //
  // Beware! Format (indent) changes may break bash-completion!
  // (see class CommandHelpFormater doc)
  //
  ///////////////////////////////////////////////////////////////////
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
  /** \overload Just option, no description
   * \code
   * "    --with-feature"
   * "    --without-feature       Common description..."
   * \endcode
   */
  CommandHelpFormater & option( boost::string_ref option_r )
  { return option( option_r, "" ); }

public:
  ///////////////////////////////////////////////////////////////////
  //
  // Beware! Format (indent) changes may break bash-completion!
  // (see class CommandHelpFormater doc)
  //
  ///////////////////////////////////////////////////////////////////
  /** Global help main section title. (0 indent)*/
  CommandHelpFormater & gMainSection( boost::string_ref text_r )
  { _mww.gotoNextPar(); _mww.writePar( text_r ); _mww.gotoNextPar(); return *this; }

  CommandHelpFormater & gMainUsage()
  { return gMainSection( _("Usage:") ); }
  CommandHelpFormater & gMainGlobalOpts()
  { return gMainSection( _("Global Options:") ); }
  CommandHelpFormater & gMainCommands()
  { return gMainSection( _("Commands:") ); }

  /** Global help section title. (2 indent) */
  CommandHelpFormater & gSection( boost::string_ref text_r = boost::string_ref() )
  { _mww.gotoNextPar(); _mww.writePar( text_r, 2 ); _mww.gotoNextPar(); return *this; }

  /** Global help Synopsis. (4 indent)*/
  CommandHelpFormater & gSynopsis( boost::string_ref text_r )
  { _mww.writePar( text_r, 4 ); return *this; }

  /** Global help definition. */
  CommandHelpFormater & gDef( boost::string_ref option_r, boost::string_ref text_r )
  { _mww.writeDefinition( option_r , text_r, (option_r.starts_with( "--" )?4:6), 28 ); return *this; }
  /** \overload Just option, no description */
  CommandHelpFormater & gDef( boost::string_ref option_r )
  { return gDef( option_r, "" ); }

private:
  std::ostringstream   _str;
  mbs::MbsWriteWrapped _mww;
};

#endif
