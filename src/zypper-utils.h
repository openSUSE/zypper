#ifndef ZYPPER_UTILS_H
#define ZYPPER_UTILS_H

#include <ostream>
#include <string>

#include <zypp/Pathname.h>
#include <zypp/Url.h>
#include <zypp/Resolvable.h>

std::string read_line_from_file( const zypp::Pathname &file );
void write_line_to_file( const zypp::Pathname &file, const std::string &line );

/**
 * Write a suggestion to report a bug to the specified stream.
 */
std::ostream & report_a_bug (std::ostream& stm);

bool is_changeable_media(const zypp::Url & url);

std::string kind_to_string_localized(
    const zypp::KindOf<zypp::Resolvable> & kind, unsigned long count);

#endif /*ZYPPER_UTILS_H*/
