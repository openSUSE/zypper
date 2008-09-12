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

/** Say the specified option has no effect */
void report_dummy_option(Out & out, const std::string & longoption_str);

/** Say you miss a required argument and print command help */
void report_required_arg_missing(Out & out, const std::string & cmd_help);

void print_usage(Out & out, const std::string & command_help);

void print_verify_hint(Out & out);


#endif /*MESSAGES_H_*/
