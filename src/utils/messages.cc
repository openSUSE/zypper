/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <sstream>
#include <zypp/base/String.h>
#include "Zypper.h"

#include "messages.h"

/// tell the user to report a bug, and how
// (multiline, with endls)
void report_a_bug( Out & out )
{
  std::ostringstream s;
  s <<_("Please file a bug report about this.") << endl
    // TranslatorExplanation remember not to translate the URL
    // unless you translate the actual page :)
    << _("See http://en.opensuse.org/Zypper/Troubleshooting for instructions.");
  out.error( s.str() );
}

// ----------------------------------------------------------------------------

void report_too_many_arguments( const std::string & specific_help )
{
  report_too_many_arguments( Zypper::instance().out(), specific_help );
}

void report_too_many_arguments( Out & out, const std::string & specific_help )
{
  //! \todo make this more explanatory, e.g. "Ingoring arg1 arg2. This command does not take arguments. See %s for more information."
  std::ostringstream s;
  s << _("Usage") << ':' << endl << specific_help;
  out.error( _("Too many arguments."), s.str() );
}

// ----------------------------------------------------------------------------

void report_alias_or_aggregate_required ( Out & out, const std::string & specific_help )
{
  // translators: aggregate option is e.g. "--all". This message will be
  // followed by mr command help text which will explain it
  out.error(_("Alias or an aggregate option is required."));
  out.info( specific_help );
}

// ----------------------------------------------------------------------------

void report_required_arg_missing( Out & out, const std::string & cmd_help )
{
  out.error(_("Required argument missing."));
  std::ostringstream s;
  s << _("Usage") << ':' << endl;
  s << cmd_help;
  out.info( s.str() );
}

// ----------------------------------------------------------------------------

void report_dummy_option(Out & out, const std::string & longoption_str)
{
  out.warning( str::form(_("The '--%s' option has currently no effect."), longoption_str.c_str()) );
}

// ----------------------------------------------------------------------------

void print_usage( Out & out, const std::string & command_help )
{
  std::ostringstream s;
  s << _("Usage") << ':' << endl;
  s << command_help;
  out.info( s.str() );
}

// ----------------------------------------------------------------------------

void print_verify_hint( Out & out )
{
  out.warning( str::form(
    _("You have chosen to ignore a problem with download or installation of a package"
    " which might lead to broken dependencies of other packages."
    " It is recommended to run '%s' after the operation has finished."),
    "zypper verify"));
}

std::string legacyCLIStr( const std::string & old_r, const std::string & new_r, LegacyCLIMsgType type_r )
{
  switch (type_r) {
  case LegacyCLIMsgType::Local:
  case LegacyCLIMsgType::Global:
    return str::Format( type_r == LegacyCLIMsgType::Global
       ? _("Legacy commandline option %1% detected. Please use global option %2% instead.")
       : _("Legacy commandline option %1% detected. Please use %2% instead.") )
       % NEGATIVEString(dashdash(old_r))
       % POSITIVEString(dashdash(new_r));
    break;
  case LegacyCLIMsgType::Ignored:
    return str::Format(
       _("Legacy commandline option %1% detected. This option is ignored."))
       % NEGATIVEString(dashdash(old_r));
    break;
  }
  return std::string();
}

void print_legacyCLIStr( Out & out, const std::string & old_r, const std::string & new_r, Out::Verbosity verbosity_r, LegacyCLIMsgType type_r )
{
  out.warning( legacyCLIStr( old_r, new_r, type_r), verbosity_r );
}
