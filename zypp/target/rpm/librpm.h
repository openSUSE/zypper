/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/librpm.h
 *
*/
#ifndef ZYPP_TARGET_RPM_LIBRPM_H
#define ZYPP_TARGET_RPM_LIBRPM_H

extern "C"
{
#include <rpm/rpmlib.h>
#include <rpm/rpmmacro.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmts.h>
#include <fcntl.h>
}

#endif // ZYPP_TARGET_RPM_LIBRPM_H
