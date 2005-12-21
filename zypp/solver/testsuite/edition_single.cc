/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* edition.cc
 *
 * Testcases for 'Edition'
 *  contains epoch-version-release-arch
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

#include <zypp/Edition.h>
#include <zypp/Arch.h>

using namespace std;
using namespace zypp;

//---------------------------------------------------------------------------

bool
defaultarch(void)
{
   const zypp::Arch arch;		// the default should be 'noarch'

   return arch == zypp::Arch_noarch;
}

bool
x86arch(void)
{
   const zypp::Arch arch("x86");

   return arch != zypp::Arch_noarch;
}

//---------------------------------------------------------------------------

bool
emptyEdition(void)
{
    Edition edition;
    return (edition.asString() == "EDITION-UNSPEC");
}


bool
versionEdition(void)
{
    Edition edition("42");
    return (edition.asString() == "42");
}

bool
releaseEdition(void)
{
    Edition edition("42","47.11");
    return (edition.asString() == "42-47.11");
}

bool
fullEdition(void)
{
    Edition edition( "42", "47.11", 1);
    return (edition.asString() == "1:42-47.11");
    
}

//---------------------------------------------------------------------------

int
main (int argc, char *argv[])
{
    struct _testcase {
	const char *name;
	bool (*fun)();
    } testcases[] = {

#define FUN(name)	{ #name, name }

	FUN(defaultarch), FUN(x86arch),
	FUN(emptyEdition),  FUN(versionEdition), FUN(releaseEdition),
        FUN(fullEdition),
	NULL, NULL

    };

    struct _testcase *testcase = testcases;

    int failed = 0;

    while (testcase->name)
    {
	bool result = (testcase->fun)();
	printf ("%-16s ", testcase->name); fflush (stdout);
	if (result) {
	    printf ("PASS\n");
	}
	else {
	    printf ("FAIL\n");
	    failed++;
	}
	testcase++;
    }

    return 0;
}


