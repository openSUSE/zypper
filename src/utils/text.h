/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_TEXT_H_
#define ZYPPER_UTILS_TEXT_H_

#include <string>
#include <iosfwd>

/** Returns the length of the string in columns */
/*
 * TODO
 * - delete whitespace at the end of lines
 * - keep one-letter words with the next
 */
unsigned string_to_columns (const std::string & str);

void wrap_text(std::ostream & out, const std::string & text,
    unsigned indent, unsigned wrap_width, int initial = -1);

#endif /* ZYPPER_UTILS_TEXT_H_ */
