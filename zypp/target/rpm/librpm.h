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

#ifdef _RPM_5
// needs to be outside 'extern "C"'
#include <rpm/rpm4compat.h>
#endif // _RPM_5

extern "C"
{
#ifdef _RPM_5
#include <rpm/rpmtag.h>
#else
#include <rpm/rpmlib.h>
#endif // _RPM_5

#include <rpm/rpmmacro.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmts.h>
#include <rpm/rpmfi.h>
#include <fcntl.h>
}

#endif // ZYPP_TARGET_RPM_LIBRPM_H
