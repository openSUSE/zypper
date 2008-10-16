/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef PAGER_H_
#define PAGER_H_

#include <string>

namespace zypp
{
  class Pathname;
}

/**
 * Opens $PAGER with given \a text. If $PAGER is not set, uses 'more' as
 * a fallback.
 *
 * \param text  Text to show.
 * \param intro Explanatory note to show at the start of the text.
 * \return true if there was no problem opening the pager
 */
bool show_text_in_pager(const std::string & text, const std::string & intro = "");

/**
 * Opens $PAGER with given \a file. If $PAGER is not set, uses 'more' as
 * a fallback.
 *
 * \param file  File to show.
 * \param intro Explanatory note to show at the start of the text.
 * \return true if there was no problem opening the pager
 */
bool show_file_in_pager(const zypp::Pathname & file, const std::string & intro = "");

#endif /*PAGER_H_*/
