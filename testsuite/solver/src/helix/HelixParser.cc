/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* HelixParser.cc  wrapper for XML I/O
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ctype.h>
#include <assert.h>
#include "HelixParser.h"
#include "HelixPackageImpl.h"
#include "HelixSourceImpl.h"

#include "zypp/ResObject.h"
#include "zypp/CapFactory.h"
#include "zypp/CapSet.h"
#include "zypp/Dependencies.h"

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

using namespace std;
using namespace zypp;

//---------------------------------------------------------------------------

static Resolvable::Kind
string2kind (const std::string & str)
{
    Resolvable::Kind kind = ResTraits<Package>::kind;
    if (!str.empty()) {
	if (str == "package") {
	    // empty
	}
	else if (str == "patch") {
	    kind = ResTraits<Patch>::kind;
	}
	else if (str == "pattern") {
	    kind = ResTraits<Pattern>::kind;
	}
	else if (str == "script") {
	    kind = ResTraits<Script>::kind;
	}
	else if (str == "message") {
	    kind = ResTraits<Message>::kind;
	}
	else if (str == "product") {
	    kind = ResTraits<Product>::kind;
	}
	else {
	    ERR << "get_resItem unknown kind '" << str << "'" << endl;
	}
    }
    return kind;
}


//---------------------------------------------------------------------------

static Capability
parse_dep_attrs(bool *is_obsolete, bool *is_pre, const xmlChar **attrs)
{
    CapFactory  factory;
    int i;
    bool op_present = false;
    /* Temporary variables dependent upon the presense of an 'op' attribute */

    Resolvable::Kind dkind = ResTraits<Package>::kind;	// default to package
    string dname;
    int depoch = Edition::noepoch;
    string dversion;
    string drelease;
    string darch = "";
    Rel relation = Rel::ANY;

    *is_obsolete = false;
    *is_pre = false;

    for (i = 0; attrs[i]; i++) {
	const string attr ((const char *)attrs[i++]);
	const string value ((const char *)attrs[i]);

	if (attr == "name")		dname = value;
	else if (attr == "kind")	dkind = string2kind (value);
	else if (attr == "op") {	op_present = true; relation = Rel(value); }
	else if (attr == "epoch")	depoch = str::strtonum<int>( value );
	else if (attr == "version")	dversion = value;
	else if (attr == "release")	drelease = value;
	else if (attr == "arch")	darch = value;
	else if (attr == "obsoletes")	*is_obsolete = true;
	else if (attr == "pre")		if (value == "1") *is_pre = true;
	else {
	    _DBG("HelixParser") << "! Unknown attribute: " << attr << " = " << value << endl;
	}

    }

    return  factory.parse ( dkind, dname, relation, Edition (dversion, drelease, depoch));
}


//---------------------------------------------------------------------------
// SAX callbacks

static void
sax_start_document(void *ptr)
{
    HelixParser *ctx = (HelixParser *)ptr;
    if (ctx->processing()) return;

//    _XXX("HelixParser") << "* Start document" << endl;

    ctx->setProcessing (true);
}


static void
sax_end_document(void *ptr)
{
    HelixParser *ctx = (HelixParser *)ptr;
    if (!ctx->processing()) return;

//    _XXX("HelixParser") << "* End document" << endl;

    ctx->setProcessing (false);
}


static void
sax_start_element(void *ptr, const xmlChar *xmlchar, const xmlChar **attrs)
{
    HelixParser *ctx = (HelixParser *)ptr;

    ctx->releaseBuffer();

#if 0
    _XXX("HelixParser") <<  "* Start element (" << (const char *)name << ")";

    if (attrs) {
	for (int i = 0; attrs[i]; i += 2) {
	    _DBG("HelixParser") <<  "   - Attribute (" << (const char *)attrs[i] << "=" << (const char *)attrs[i+1] << ")" << endl;
	}
    }
#endif

    string token ((const char *)xmlchar);

    if (token == "channel"
	|| token == "subchannel") {
	/* Unneeded container tags.  Ignore */
	return;
    }

    return ctx->startElement (token, attrs);

}


static void
sax_end_element(void *ptr, const xmlChar *xmlchar)
{
    HelixParser *ctx = (HelixParser *)ptr;

//    _XXX("HelixParser") << "* End element (" << (const char *)name << ")" << endl;
    string token ((const char *)xmlchar);

    if (token == "channel"
	|| token == "subchannel") {
	/* Unneeded container tags.  Ignore */
	token.clear();
    }

    return ctx->endElement (token);
}


static void
sax_characters(void *ptr, const xmlChar *ch, int len)
{
    HelixParser *ctx = (HelixParser *)ptr;

    ctx->toBuffer (str::trim (string ((const char *)ch, len)));
    return;
}


static void
sax_warning(void *ptr, const char *msg, ...)
{
    va_list args;
    char tmp[2048];

    va_start(args, msg);

    if (vsnprintf(tmp, 2048, msg, args) >= 2048) ERR << "vsnprintf overflow" << endl;
    WAR << "* SAX Warning: " << tmp << endl;

    va_end(args);
}


static void
sax_error(void *ptr, const char *msg, ...)
{
    va_list args;
    char tmp[2048];

    va_start(args, msg);

    if (vsnprintf(tmp, 2048, msg, args) >= 2048) ERR << "vsnprintf overflow" << endl;;
    ERR << "* SAX Error: " <<  tmp <<endl;

    va_end(args);
}


static xmlSAXHandler sax_handler = {
    NULL,		/* internalSubset */
    NULL,		/* isStandalone */
    NULL,		/* hasInternalSubset */
    NULL,		/* hasExternalSubset */
    NULL,		/* resolveEntity */
    NULL,		/* getEntity */
    NULL,		/* entityDecl */
    NULL,		/* notationDecl */
    NULL,		/* attributeDecl */
    NULL,		/* elementDecl */
    NULL,		/* unparsedEntityDecl */
    NULL,		/* setDocumentLocator */
    sax_start_document,	/* startDocument */
    sax_end_document,	/* endDocument */
    sax_start_element,	/* startElement */
    sax_end_element,	/* endElement */
    NULL,		/* reference */
    sax_characters,     /* characters */
    NULL,		/* ignorableWhitespace */
    NULL,		/* processingInstruction */
    NULL,		/* comment */
    sax_warning,	/* warning */
    sax_error,		/* error */
    sax_error		/* fatalError */
};

//---------------------------------------------------------------------------

HelixParser::HelixParser ()
    : _processing (false)
    , _xml_context (NULL)
    , _state (PARSER_TOPLEVEL)
    , _stored(true)
    , _dep_set (NULL)
    , _toplevel_dep_set (NULL)
    , _impl (NULL)
{
//    _XXX("HelixParser") <<  "* Context created (" << this << ")" << endl;
}


HelixParser::~HelixParser()
{
    releaseBuffer ();
}

//---------------------------------------------------------------------------

string
HelixParser::asString ( void ) const
{
    return toString (*this);
}


string
HelixParser::toString ( const HelixParser & context )
{
    return "<HelixParser/>";
}


ostream &
HelixParser::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const HelixParser& context)
{
    return os << context.asString();
}

//---------------------------------------------------------------------------

void
HelixParser::toBuffer (string data)
{
    _text_buffer = data;

//    _XXX("HelixParser") << "HelixParser[" << this << "]::toBuffer(" << data << "...," << (long)size << ")" << endl;
}


void
HelixParser::releaseBuffer ()
{
    _text_buffer.clear();

//    _XXX("HelixParser") <<  "HelixParser[" << this << "]::releaseBuffer()" << endl;
}


// called for extracting

void
HelixParser::parseChunk(const char *xmlbuf, size_t size, HelixSourceImpl *impl)
{
//    _DBG("HelixParser") << "HelixParser::parseChunk(" << xmlbuf << "...," << (long)size << ")" << endl;

    xmlSubstituteEntitiesDefault(true);

    if (!_xml_context) {
	_xml_context = xmlCreatePushParserCtxt(&sax_handler, this, NULL, 0, NULL);
    }

    _impl = impl;

    xmlParseChunk(_xml_context, xmlbuf, size, 0);
}


void
HelixParser::done()
{
//    _XXX("HelixParser") << "HelixParser::done()" << endl;

    if (_processing)
	xmlParseChunk(_xml_context, NULL, 0, 1);

    if (_xml_context)
	xmlFreeParserCtxt(_xml_context);

    if (!_stored) {
	ERR << "Incomplete package lost" << endl;
    }

    return;
}


//---------------------------------------------------------------------------
// Parser state callbacks

void
HelixParser::startElement(const string & token, const xmlChar **attrs)
{
//    _XXX("HelixParser") << "HelixParser::startElement(" << token << ")" << endl;

    switch (_state) {
	case PARSER_TOPLEVEL:
	    toplevelStart (token, attrs);
	    break;
	case PARSER_RESOLVABLE:
	    resolvableStart (token, attrs);
	    break;
	case PARSER_HISTORY:
	    historyStart (token, attrs);
	    break;
	case PARSER_DEP:
	    dependencyStart (token, attrs);
	    break;
	default:
	    break;
    }

    return;
}


void
HelixParser::endElement(const string & token)
{
//    _XXX("HelixParser") <<  "HelixParser::endElement(" << token << ")" << endl;

    if (!token.empty()) {			// sax_end_element might set token to NULL
	switch (_state) {
	    case PARSER_RESOLVABLE:
		resolvableEnd (token);
		break;
	    case PARSER_HISTORY:
		historyEnd (token);
		break;
	    case PARSER_UPDATE:
		updateEnd (token);
		break;
	    case PARSER_DEP:
		dependencyEnd (token);
		break;
	    default:
		break;
	}
    }

    releaseBuffer();

    return;
}


void
HelixParser::toplevelStart(const std::string & token, const xmlChar **attrs)
{
//    _XXX("HelixParser") << "HelixParser::toplevelStart(" << token << ")" << endl;

    if ((token == "package")
	|| (token == "pattern")
	|| (token == "script")
	|| (token == "message")
	|| (token == "patch")
	|| (token == "product")) {

	_state = PARSER_RESOLVABLE;

	_stored = false;
	name = "";
	summary = "";
	description = "";
	section = "";
	kind = string2kind (token);	// needed for <update>..</update>, see updateStart

	fileSize = 0;
	installedSize = 0;
	installOnly = false;
	installed = false;

	requires.clear();
	provides.clear();
	conflicts.clear();
	freshens.clear();
	enhances.clear();
	recommends.clear();
	suggests.clear();
	obsoletes.clear();

    }
    else {
	_DBG("HelixParser") << "! Not handling " << token << " at toplevel" << endl;
    }
}


void
HelixParser::resolvableStart(const std::string & token, const xmlChar **attrs)
{
//    _XXX("HelixParser") << "HelixParser::resolvableStart(" << token << ")" << endl;

    /* Only care about the containers here */
    if (token == "history") {
	_state = PARSER_HISTORY;
    }
    else if (token == "deps") {
	/*
	 * We can get a <deps> tag surrounding the actual package
	 * dependency sections (requires, provides, conflicts, etc).
	 * In this case, we'll just ignore this tag quietly.
	 */
    }
    else if (token == "requires") {
	_state = PARSER_DEP;
	_dep_set = _toplevel_dep_set = &requires;
    }
    else if (token == "recommends") {
	_state = PARSER_DEP;
	_dep_set = _toplevel_dep_set = &recommends;
    }
    else if (token == "suggests") {
	_state = PARSER_DEP;
	_dep_set = _toplevel_dep_set = &suggests;
    }
    else if (token == "conflicts") {
	bool is_obsolete = false;
	int i;

	_state = PARSER_DEP;

	for (i = 0; attrs && attrs[i] && !is_obsolete; i += 2) {
	    string attr ((const char *)(attrs[i]));
	    if (attr == "obsoletes")
		is_obsolete = true;
	}

	if (is_obsolete)
	    _dep_set = _toplevel_dep_set = &obsoletes;
	else {
	    _dep_set = _toplevel_dep_set = &conflicts;
	}
    }
    else if (token == "obsoletes") {
	_state = PARSER_DEP;
	_dep_set = _toplevel_dep_set = &obsoletes;
    }
    else if (token == "provides") {
	_state = PARSER_DEP;
	_dep_set = _toplevel_dep_set = &provides;
    }
#if 0		// disabled
    else if (token == "children") {
	_state = PARSER_DEP;
	_dep_set = _toplevel_dep_set = &children;
    }
#endif
    else if (token == "freshens") {
	_state = PARSER_DEP;
	_dep_set = _toplevel_dep_set = &freshens;
    }
    else if (token == "enhances") {
	_state = PARSER_DEP;
	_dep_set = _toplevel_dep_set = &enhances;
    }
    else {
//	_XXX("HelixParser") << "! Not handling " << token << " in package start" << endl;
    }
}


void
HelixParser::historyStart(const std::string & token, const xmlChar **attrs)
{
//    _XXX("HelixParser") << "HelixParser::historyStart(" << token << ")" << endl;

    if (token == "update") {

	_state = PARSER_UPDATE;
    }
    else {
	_XXX("HelixParser") << "! Not handling " << token << " in history" << endl;
    }
}


void
HelixParser::dependencyStart(const std::string & token, const xmlChar **attrs)
{
//    _XXX("HelixParser") << "HelixParser::dependencyStart(" << token << ")" << endl;

    if (token == "dep") {
	Capability dep;
	bool is_obsolete;
	bool is_pre;

	dep = parse_dep_attrs(&is_obsolete, &is_pre, attrs);

	if (is_obsolete)
	    obsoletes.insert (dep);
	else if (is_pre)
	    prerequires.insert (dep);
	else {
	    _dep_set->insert (dep);
	}
    }
    else if (token == "or")
	_dep_set->clear();
    else {
	_DBG("HelixParser") <<  "! Not handling " << token << " in dependency" << endl;
    }
}


//---------------------------------------------------------------------------


void
HelixParser::resolvableEnd (const std::string & token)
{
//    _XXX("HelixParser") << "HelixParser::resolvableEnd(" << token << ")" << endl;

    if ((token == "package")
	|| (token == "pattern")
	|| (token == "script")
	|| (token == "message")
	|| (token == "patch")
	|| (token == "product")) {

	kind = string2kind (token);

	// check if we provide ourselfs properly

	CapFactory  factory;
	Edition edition (version, release, epoch);
	Capability selfdep = factory.parse ( kind, name, Rel::EQ, edition);
	CapSet::const_iterator piter;
	for (piter = provides.begin(); piter != provides.end(); piter++) {
	    if ((*piter) == selfdep)
	    {
		break;
	    }
	}

	if (piter == provides.end()) {			// no self provide found, construct one
//	    _XXX("HelixParser") << "Adding self-provide [" << selfdep.asString() << "]" << endl;
	    provides.insert (selfdep);
	}

#warning Needs to know if a source is actually the target
#if 0
	if (_channel->system())
	    installed = true;
#endif
	if (_impl) {
	    _impl->parserCallback (*this);
	}

//	_DBG("HelixParser") << package->asString(true) << endl;
//	_DBG("HelixParser") << "HelixParser::resolvableEnd(" << token << ") done: '" << package->asString(true) << "'" << endl;
//	_XXX("HelixParser") << "HelixParser::resolvableEnd now " << _all_packages.size() << " packages" << endl;
	_stored = true;
	_state = PARSER_TOPLEVEL;
    }
    else if (token == "name") {			name = _text_buffer;
    } else if (token == "pretty_name") {	// ignore
    } else if (token == "summary") {		summary = _text_buffer;
    } else if (token == "description") {	description = _text_buffer;
    } else if (token == "section") {		section = _text_buffer;
    } else if (token == "arch") {		arch = _text_buffer;
    } else if (token == "filesize") {		fileSize = str::strtonum<long>(_text_buffer);
    } else if (token == "installedsize") {	installedSize = str::strtonum<long>(_text_buffer);
    } else if (token == "install_only") {	installOnly = true;
    } else if (token == "md5sum") {		// ignore
    } else {
	_DBG("HelixParser") << "HelixParser::resolvableEnd(" << token << ") unknown" << endl;
    }

//    _XXX("HelixParser") << "HelixParser::resolvableEnd(" << token << ") done" << endl;

    releaseBuffer();
}


void
HelixParser::historyEnd (const std::string & token)
{
//    _XXX("HelixParser") << "HelixParser::historyEnd(" << token << ")" << endl;

    if (token == "history") {

	_state = PARSER_RESOLVABLE;
    }
}


void
HelixParser::updateEnd (const std::string & token)
{
//    _XXX("HelixParser") << "HelixParser::updateEnd(" << token << ")" << endl;

    if (token == "update") {
	_state = PARSER_HISTORY;
    } else if (token == "epoch") {		epoch = str::strtonum<int>(_text_buffer);
    } else if (token == "version") {		version = _text_buffer;
    } else if (token == "release") {		release = _text_buffer;
    } else if (token == "arch") {		arch = _text_buffer;
    } else if (token == "filename") {		// ignore
    } else if (token == "filesize") {		fileSize = str::strtonum<long>(_text_buffer);
    } else if (token == "installedsize") {	installedSize = str::strtonum<long>(_text_buffer);
    } else if (token == "signaturename") {	// ignore
    } else if (token == "signaturesize") {	// ignore
    } else if (token == "md5sum") {		// ignore
    } else if (token == "importance") {		// ignore
    } else if (token == "description") {	// ignore
    } else if (token == "hid") {		// ignore
    } else if (token == "license") {		// ignore
    } else {
	ERR << "HelixParser::updateEnd(" << token << ") unknown" << endl;
    }

    releaseBuffer();
}


void
HelixParser::dependencyEnd (const std::string & token)
{
//    _XXX("HelixParser") << "HelixParser::dependencyEnd(" << token << ")" << endl;

    if (token == "or") {
#if 0
	OrDependency_Ptr or_dep = OrDependency::fromDependencyList (*_dep_set);
	Dependency_Ptr dep = new Dependency (or_dep);

	(*_dep_set).clear();

	(*_toplevel_dep_set).push_back (dep);
	_dep_set = _toplevel_dep_set;
#endif
    }
    else if (token == "dep") {
	/* We handled everything we needed for dep in start */
    }
    else {
	/* All of the dep lists (requires, provides, etc.) */
	_toplevel_dep_set = NULL;
	_dep_set = NULL;
	_state = PARSER_RESOLVABLE;
    }
}

