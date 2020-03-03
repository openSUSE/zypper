/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include "output/Out.h"


/** Write a suggestion to report a bug to the output. */
void report_a_bug (Out & out);

/** Say that too many arguments have been specified */
void report_too_many_arguments(const std::string & specific_help); // deprecated
void report_too_many_arguments(Out & out, const std::string & specific_help);

/** Say that too few arguments have been specified */
void report_too_few_arguments( const std::string & specific_help );
void report_too_few_arguments( Out & out, const std::string & specific_help );

/** Say the that either a aggregate option or a alias is required */
void report_alias_or_aggregate_required ( Out & out, const std::string & specific_help );

/** Say the specified option has no effect */
void report_dummy_option(Out & out, const std::string & longoption_str);

/** Say you miss a required argument and print command help */
void report_required_arg_missing(Out & out, const std::string & cmd_help);

void print_usage(Out & out, const std::string & command_help);

void print_verify_hint(Out & out);

/** Prints a error about deprecated rug compatibility and exists */
void exit_rug_compat ();

enum class LegacyCLIMsgType {
  Local,
  Global,
  Ignored,
  Abbreviated
};
std::string legacyCLIStr( const std::string & old_r, const std::string & new_r, LegacyCLIMsgType type_r );
void print_legacyCLIStr(Out & out, const std::string & old_r, const std::string & new_r, Out::Verbosity verbosity_r = Out::NORMAL, LegacyCLIMsgType type_r = LegacyCLIMsgType::Local );

void print_unknown_command_hint( Zypper & zypper, const std::string & cmd_r );



#endif /*MESSAGES_H_*/
