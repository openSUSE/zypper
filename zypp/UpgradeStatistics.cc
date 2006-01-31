/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/* UpgradeStatistics.cc
 *
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/*
  stolen from PMPackageManager_update.cc
  original author Michael Andres <ma@suse.de>
  zypp port by Klaus Kaempf <kkaempf@suse.de>

/-*/

#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/UpgradeStatistics.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////

using namespace std;

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
**
**	DESCRIPTION :
*/
std::ostream &
UpgradeStatistics::dumpOn( std::ostream & str ) const
{
  str << "===[options]========================================" << endl;
  str << "delete_unmaintained  " << delete_unmaintained << endl;
  str << "===[initial]========================================" << endl;
  str << "pre_todel            " << pre_todel << endl;
  str << "pre_nocand           " << pre_nocand << endl;
  str << "pre_avcand           " << pre_avcand << endl;
  str << "===[checks]=========================================" << endl;
  str << "chk_installed_total  " << chk_installed_total << endl;
  str << endl;
  str << "chk_already_todel    " << chk_already_todel << endl;
  str << "chk_is_taboo         " << chk_is_taboo << endl;
  str << endl;
  str << "chk_already_toins    " << chk_already_toins << endl;
  str << "chk_to_update        " << chk_to_update << endl;
  str << "chk_to_downgrade     " << chk_to_downgrade << endl;
  str << "chk_to_keep_downgrade" << chk_to_keep_downgrade << endl;
  str << "chk_to_keep_installed" << chk_to_keep_installed << endl;
  str << "--------------------------" << endl;
  str << "avcand               "
    <<  ( chk_already_toins + chk_to_update + chk_to_downgrade + chk_to_keep_downgrade + chk_to_keep_installed )
      << endl;
  str << endl;
  str << "chk_keep_foreign     " << chk_keep_foreign << endl;
  str << "chk_dropped          " << chk_dropped << endl;
  str << "chk_replaced         " << chk_replaced << endl;
  str << "chk_replaced_guessed " << chk_replaced_guessed << endl;
  str << "chk_add_split        " << chk_add_split << endl;
  str << "--------------------------" << endl;
  str << "nocand               "
    <<  ( chk_keep_foreign + chk_dropped + chk_replaced + chk_replaced_guessed + chk_add_split )
      << endl;
  str << "===[sum]============================================" << endl;
  str << "Packages checked     " << chk_installed_total << endl;
  str << endl;
  str << "totalToInstall       " << totalToInstall() << endl;
  str << "totalToDelete        " << totalToDelete() << endl;
  str << "totalToKeep          " << totalToKeep() << endl;
  str << "--------------------------" << endl;
  str << "sum                  "
    <<  ( totalToInstall() + totalToDelete() + totalToKeep() )
      << endl;
  str << "====================================================" << endl;
  str << "====================================================" << endl;

  return str;
}


  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////


