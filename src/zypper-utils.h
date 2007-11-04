#ifndef ZYPPER_UTILS_H
#define ZYPPER_UTILS_H

#include <ostream>
#include <string>

#include <zypp/Url.h>
#include <zypp/Resolvable.h>

/**
 * Write a suggestion to report a bug to the specified stream.
 */
std::ostream & report_a_bug (std::ostream& stm);

bool is_changeable_media(const zypp::Url & url);

std::string kind_to_string_localized(
    const zypp::KindOf<zypp::Resolvable> & kind, unsigned long count);

/**
 * Constructor wrapper catching exceptions,
 * returning an empty one on error.
 */
zypp::Url make_url (const std::string & url_s);

#endif /*ZYPPER_UTILS_H*/
