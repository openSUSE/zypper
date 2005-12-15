/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* spec.cc
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Testcases for 'Spec'
 *  contains name + Edition
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

#include <zypp/solver/detail/Spec.h>

using namespace std;
using namespace zypp::solver::detail;

bool
emptySpec (void)
{
    Spec spec(Kind::Package, "");
    return spec.asString() == "";
}

bool
epochSpec(void)
{
    Spec spec(Kind::Package, "foo", 1);

    return (spec.asString() == "foo-1:");
}

bool
versionSpec(void)
{
    Spec spec(Kind::Patch, "bar", 0,"42");
    return (spec.asString() == "patch:bar-0:42");
}

bool
releaseSpec(void)
{
    Spec spec(Kind::Package, "foobar", -1,"42","47.11");

    return (spec.asString() == "foobar-42-47.11");
}

bool
archSpec(void)
{
    Spec spec(Kind::Package, "arch", 0, "42", "47.11", Arch::create("x86_64"));

    return (spec.asString() == "arch-0:42-47.11.x86_64");
}

bool
fullSpec(void)
{
    Spec spec(Kind::Package, "full", 1, "42", "47.11", Arch::create("x86_64"));

    return (spec.asString() == "full-1:42-47.11.x86_64");
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

	FUN(emptySpec), FUN(epochSpec), FUN(versionSpec), FUN(releaseSpec), FUN(archSpec), FUN(fullSpec), 
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
