/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file	zypp/ui/UserWantedPackages.h
 *
 *  \author	Stefan Hundhammer <sh@suse.de>
 *
 */
#ifndef USER_WANTED_PACKAGES_H
#define USER_WANTED_PACKAGES_H

#include <set>
#include <string>

namespace zypp
{
    namespace ui
    {
	/**
	 * This returns a set of package names the user explicitly wanted to
	 * transact ( to install, to update, or to delete) for any of the
	 * following reasons:
	 *
	 * - The user wanted to transact the pkg directly
	 *
	 * - Pkg is part of a pattern   the user wanted to transact
	 * - Pkg is part of a language  the user wanted to transact
         *   (? No more transacting Languages)
	 * - Pkg is part of a patch     the user wanted to transact
	 *
	 * - Pkg is part of a pattern that is required by a pattern the
	 *   user wanted to transact
	 **/
	std::set<std::string> userWantedPackageNames();

    } // namespace ui
} // namespace zypp

#endif // USER_WANTED_PACKAGES_H
