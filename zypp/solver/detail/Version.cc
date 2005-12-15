/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Version.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
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

#include <y2util/stringutil.h>

#include <zypp/solver/detail/Version.h>

#include <zypp/solver/detail/debug.h>

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      Version GVersion;
      
      using namespace std;
      
      string
      Version::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      Version::toString ( const Version & section )
      {
          return "<version/>";
      }
      
      ostream &
      Version::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const Version& section)
      {
          return os << section.asString();
      }
      
      //---------------------------------------------------------------------------
      
      static EditionPtr
      rpm_parse (const char *input)
      {
          const char *vptr = NULL, *rptr = NULL;
      
          if (input == NULL || *input == 0)
      	return NULL;
      
          int epoch = -1;
          const char *version;
          const char *release;
          const char *arch = "";
      
          if ((vptr = strchr (input, ':'))) {
      	/* We -might- have an epoch here */
      	char *endptr;
      
      	epoch = strtoul (input, &endptr, 10);
      
      	if (endptr != vptr) {
      	    /* No epoch here, just a : in the version string */
      	    epoch = -1;
      	    vptr = input;
      	} else {
      	    vptr++;
      	}
          } else {
      	vptr = input;
          }
      
          if ((rptr = strchr (vptr, '-'))) {
      	char *v = strndup (vptr, rptr - vptr);
      	version = v;
      	release = rptr + 1;
          } else {
      	version = vptr;
      	release = NULL;
          }
      
          EditionPtr edition = new Edition(epoch, version, release, Arch::create(arch));
      
          return edition;
      }
      
      
      /* This was stolen from RPM */
      /* And then slightly hacked on by me */
      /* And then hacked on more by me */
      
      /* compare alpha and numeric segments of two versions */
      /* return 1: a is newer than b */
      /*        0: a and b are the same version */
      /*       -1: b is newer than a */
      static int
      vercmp (const char *a, const char *b)
      {
          char oldch1, oldch2;
          char * str1, * str2;
          char * one, * two;
          int rc;
          int isnum;
          unsigned int alen, blen;
      
          /* easy comparison to see if versions are identical */
          if (!strcmp(a, b)) return 0;
      
          alen = strlen (a);
          blen = strlen (b);
      
          str1 = (char *)alloca(alen + 1);
          str2 = (char *)alloca(blen + 1);
      
          strcpy(str1, a);
          strcpy(str2, b);
      
          one = str1;
          two = str2;
      
          /* loop through each version segment of str1 and str2 and compare them */
          while (*one && *two) {
      	while (*one && !isalnum(*one)) one++;
      	while (*two && !isalnum(*two)) two++;
      
      	str1 = one;
      	str2 = two;
      
      	/* grab first completely alpha or completely numeric segment */
      	/* leave one and two pointing to the start of the alpha or numeric */
      	/* segment and walk str1 and str2 to end of segment */
      	if (isdigit(*str1)) {
      	    while (*str1 && isdigit(*str1)) str1++;
      	    while (*str2 && isdigit(*str2)) str2++;
      	    isnum = 1;
      	} else {
      	    while (*str1 && isalpha(*str1)) str1++;
      	    while (*str2 && isalpha(*str2)) str2++;
      	    isnum = 0;
      	}
      
      	/* save character at the end of the alpha or numeric segment */
      	/* so that they can be restored after the comparison */
      	oldch1 = *str1;
      	*str1 = '\0';
      	oldch2 = *str2;
      	*str2 = '\0';
      
      	/* This should only happen if someone is changing the string */
      	/* behind our back.  It should be a _very_ rare race condition */
      	if (one == str1) return -1; /* arbitrary */
      
      	/* take care of the case where the two version segments are */
      	/* different types: one numeric and one alpha */
      
      	/* Here's how we handle comparing numeric and non-numeric
      	 * segments -- non-numeric (ximian.1) always sorts lower than
      	 * numeric (0.ximian.6.1). */
      	if (two == str2)
      	    return (isnum ? 1 : -1);
      
      	if (isnum) {
      	    /* this used to be done by converting the digit segments */
      	    /* to ints using atoi() - it's changed because long  */
      	    /* digit segments can overflow an int - this should fix that. */
      
      	    /* throw away any leading zeros - it's a number, right? */
      	    while (*one == '0') one++;
      	    while (*two == '0') two++;
      
      	    /* whichever number has more digits wins */
      	    if (strlen(one) > strlen(two)) return 1;
      	    if (strlen(two) > strlen(one)) return -1;
      	}
      
      	/* strcmp will return which one is greater - even if the two */
      	/* segments are alpha or if they are numeric.  don't return  */
      	/* if they are equal because there might be more segments to */
      	/* compare */
      	rc = strcmp(one, two);
      	if (rc) return rc;
      
      	/* restore character that was replaced by null above */
      	*str1 = oldch1;
      	one = str1;
      	*str2 = oldch2;
      	two = str2;
          }
      
          /* this catches the case where all numeric and alpha segments have */
          /* compared identically but the segment sepparating characters were */
          /* different */
          if ((!*one) && (!*two)) return 0;
      
          /* whichever version still has characters left over wins */
          if (!*one) return -1; else return 1;
      }
      
      
      static int
      rpm_compare (constSpecPtr spec1, constSpecPtr spec2)
      {
          int rc = 0;
      
          assert (spec1 != NULL);
          assert (spec2 != NULL);
      
          const string name1 = spec1->name();
          const string name2 = spec2->name();
          if (! (name1.empty() && name2.empty()))
          {
      	rc = name1.compare (name2);
          }
          if (rc) return rc;
          
          if (spec1->epoch() >= 0 && spec2->epoch() >= 0) {
      	rc = spec1->epoch() - spec2->epoch();
          } else if (spec1->epoch() > 0) {
      	rc = 1;
          } else if (spec2->epoch() > 0) {
      	rc = -1;
          }
          if (rc) return rc;
      
          rc = vercmp (spec1->version().c_str(), spec2->version().c_str());
          if (rc) return rc;
      
          const string rel1 = spec1->release();
          const string rel2 = spec2->release();
          if (!rel1.empty() && !rel2.empty()) {
      	rc = vercmp (rel1.c_str(), rel2.c_str());
          }
          return rc;
      }
      
      //---------------------------------------------------------------------------
      
      Version::Version()
          : _properties (VERSION_PROP_PROVIDE_ANY | VERSION_PROP_IGNORE_ABSENT_EPOCHS)
          , _parse (rpm_parse)
          , _compare (rpm_compare)
      {
      }
      
      
      Version::~Version()
      {
      }
      
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

