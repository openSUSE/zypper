/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* XmlParser.cc  wrapper for XML I/O
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
#include "zypp/solver/temporary/XmlParser.h"
#include "zypp/solver/temporary/PackageUpdate.h"
#include "zypp/solver/temporary/utils.h"
#include "zypp/ResObject.h"
#include "zypp/CapFactory.h"
#include "zypp/CapSet.h"
#include "zypp/Dependencies.h"
#include "zypp/base/Logger.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
    namespace solver
    { /////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////
  namespace detail
  { ///////////////////////////////////////////////////////////////////

using namespace std;

//---------------------------------------------------------------------------

static Resolvable::Kind
string2kind (const std::string & str)
{
    Resolvable::Kind kind = ResTraits<zypp::Package>::kind;
    if (!str.empty()) {
	if (str == "package") {
	    // empty
	}
	else if (str == "patch") {
	    kind = ResTraits<zypp::Patch>::kind;
	}
	else if (str == "pattern") {
	    kind = ResTraits<zypp::Pattern>::kind;
	}
	else if (str == "script") {
	    kind = ResTraits<zypp::Script>::kind;
	}
	else if (str == "message") {
	    kind = ResTraits<zypp::Message>::kind;
	}
	else if (str == "product") {
	    kind = ResTraits<zypp::Product>::kind;
	}
	else {
	    ERR << "get_resItem unknown kind '" << str << "'" << endl;
	}
    }
    return kind;
}


//---------------------------------------------------------------------------

static Capability
parse_dep_attrs(bool *is_obsolete, const xmlChar **attrs)
{
    CapFactory  factory;
    int i;
    bool op_present = false;
    /* Temporary variables dependent upon the presense of an 'op' attribute */
    Resolvable::Kind kind = ResTraits<zypp::Package>::kind;
    const char *name = NULL;
    int epoch = Edition::noepoch;
    string version;
    string release;
    string arch = "";
    Rel relation = Rel::ANY;

    *is_obsolete = false;

    for (i = 0; attrs[i]; i++) {
	const char *attr = (const char *)attrs[i++];
	const char *value = (const char *)attrs[i];

	if (!strcasecmp(attr, "name"))		    name = value;
	else if (!strcasecmp(attr, "kind"))	    kind = string2kind (value);
	else if (!strcasecmp(attr, "op"))	{   op_present = true; relation = Rel(value); }
	else if (!strcasecmp(attr, "epoch"))	    epoch = atoi (value);
	else if (!strcasecmp(attr, "version"))      version = value;
	else if (!strcasecmp(attr, "release"))	    release = value;
	else if (!strcasecmp(attr, "arch"))	    arch = value;
	else if (!strcasecmp (attr, "obsoletes"))   *is_obsolete = true;
	else {
	    _DBG("RC_SPEW_XML") << "! Unknown attribute: " << attr << " = " << value << endl;
	}

    }

    /* FIXME: should get Channel from XML */
    return  factory.parse ( kind, name, relation, Edition (version, release, epoch));
}


//---------------------------------------------------------------------------
// SAX callbacks

static void
sax_start_document(void *ptr)
{
    XmlParser *ctx = (XmlParser *)ptr;
    if (ctx->processing()) return;

//    _XXX("RC_SPEW_XML") << "* Start document" << endl;

    ctx->setProcessing (true);
}


static void
sax_end_document(void *ptr)
{
    XmlParser *ctx = (XmlParser *)ptr;
    if (!ctx->processing()) return;

//    _XXX("RC_SPEW_XML") << "* End document" << endl;

    ctx->setProcessing (false);
}


static void
sax_start_element(void *ptr, const xmlChar *name, const xmlChar **attrs)
{
    XmlParser *ctx = (XmlParser *)ptr;

    ctx->releaseBuffer();

#if 0
    _XXX("RC_SPEW_XML") <<  "* Start element (" << (const char *)name << ")";

    if (attrs) {
	for (int i = 0; attrs[i]; i += 2) {
	    _DBG("RC_SPEW_XML") <<  "   - Attribute (" << (const char *)attrs[i] << "=" << (const char *)attrs[i+1] << ")" << endl;
	}
    }
#endif
    if (!strcmp((const char *)name, "channel") || !strcmp((const char *)name, "subchannel")) {
	/* Unneeded container tags.  Ignore */
	return;
    }

    return ctx->startElement ((const char *)name, attrs);

}


static void
sax_end_element(void *ptr, const xmlChar *name)
{
    XmlParser *ctx = (XmlParser *)ptr;

//    _XXX("RC_SPEW_XML") << "* End element (" << (const char *)name << ")" << endl;

    if (!strcmp((const char *)name, "channel") || !strcmp((const char *)name, "subchannel")) {
	/* Unneeded container tags.  Ignore */
	name = NULL;
    }

    return ctx->endElement ((const char *)name);
}


static void
sax_characters(void *ptr, const xmlChar *ch, int len)
{
    XmlParser *ctx = (XmlParser *)ptr;

    ctx->toBuffer ((const char *)ch, len);
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

XmlParser::XmlParser (Channel_constPtr channel)
    : _channel (channel)
    , _processing (false)
    , _xml_context (NULL)
    , _state (PARSER_TOPLEVEL)
    , _current_resitem_stored(true)
    , _current_update (NULL)
    , _toplevel_dep_list (NULL)
    , _current_dep_list (NULL)
    , _text_buffer (NULL)
    , _text_buffer_size (0)
{
//    _XXX("RC_SPEW_XML") <<  "* Context created (" << this << ")" << endl;
}


XmlParser::~XmlParser()
{
    releaseBuffer ();
}

//---------------------------------------------------------------------------

string
XmlParser::asString ( void ) const
{
    return toString (*this);
}


string
XmlParser::toString ( const XmlParser & context )
{
    return "<XmlParser/>";
}


ostream &
XmlParser::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const XmlParser& context)
{
    return os << context.asString();
}

//---------------------------------------------------------------------------

void
XmlParser::toBuffer (const char *data, size_t size)
{
    _text_buffer = (char *)realloc (_text_buffer, _text_buffer_size + size + 1);
    strncpy (_text_buffer + _text_buffer_size, (char *)data, size);
    _text_buffer_size += size;
    _text_buffer[_text_buffer_size] = 0;

//    _XXX("RC_SPEW_XML") << "XmlParser[" << this << "]::toBuffer(" << data << "...," << (long)size << ")" << endl;
}


void
XmlParser::releaseBuffer ()
{
    if (_text_buffer)
	free (_text_buffer);
    _text_buffer = NULL;
    _text_buffer_size = 0;
//    _XXX("RC_SPEW_XML") <<  "XmlParser[" << this << "]::releaseBuffer()" << endl;
}


void
XmlParser::parseChunk(const char *xmlbuf, size_t size)
{
//    _DBG("RC_SPEW_XML") << "XmlParser::parseChunk(" << xmlbuf << "...," << (long)size << ")" << endl;

    xmlSubstituteEntitiesDefault(true);

    if (!_xml_context) {
	_xml_context = xmlCreatePushParserCtxt(&sax_handler, this, NULL, 0, NULL);
    }

    xmlParseChunk(_xml_context, xmlbuf, size, 0);
}


PackageList
XmlParser::done()
{
//    _XXX("RC_SPEW_XML") << "XmlParser::done()" << endl;

    if (_processing)
	xmlParseChunk(_xml_context, NULL, 0, 1);

    if (_xml_context)
	xmlFreeParserCtxt(_xml_context);

    if (!_current_resitem_stored) {
	ERR << "Incomplete package lost" << endl;
    }

    if (_current_update) {
	ERR << "Incomplete update lost" << endl;
    }

    return _all_packages;
}


//---------------------------------------------------------------------------
// Parser state callbacks

void
XmlParser::startElement(const char *name, const xmlChar **attrs)
{
//    _XXX("RC_SPEW_XML") << "XmlParser::startElement(" << name << ")" << endl;

    switch (_state) {
	case PARSER_TOPLEVEL:
	    toplevelStart (name, attrs);
	    break;
	case PARSER_RESOLVABLE:
	    resolvableStart (name, attrs);
	    break;
	case PARSER_HISTORY:
	    historyStart (name, attrs);
	    break;
	case PARSER_DEP:
	    dependencyStart (name, attrs);
	    break;
	default:
	    break;
    }

    return;
}


void
XmlParser::endElement(const char *name)
{
//    _XXX("RC_SPEW_XML") <<  "XmlParser::endElement(" << name << ")" << endl;

    if (name != NULL) {			// sax_end_element might set name to NULL
	switch (_state) {
	    case PARSER_RESOLVABLE:
		resolvableEnd (name);
		break;
	    case PARSER_HISTORY:
		historyEnd (name);
		break;
	    case PARSER_UPDATE:
		updateEnd (name);
		break;
	    case PARSER_DEP:
		dependencyEnd (name);
		break;
	    default:
		break;
	}
    }

    releaseBuffer();

    return;
}


void
XmlParser::toplevelStart(const std::string & name, const xmlChar **attrs)
{
//    _XXX("RC_SPEW_XML") << "XmlParser::toplevelStart(" << name << ")" << endl;

    if ((name == "package")
	|| (name == "pattern")
	|| (name == "script")
	|| (name == "message")
	|| (name == "patch")
	|| (name == "product")) {

	_state = PARSER_RESOLVABLE;

	_current_resitem_stored = false;
	_current_resitem_name = "";
	_current_resitem_prettyName = "";
	_current_resitem_summary = "";
	_current_resitem_description = "";
	_current_resitem_section = "";
	_current_resitem_kind = string2kind (name);	// needed for <update>..</update>, see updateStart
	_current_resitem_arch = Arch_noarch;
	_current_resitem_edition = Edition::noedition;
	_current_resitem_fileSize = 0;
	_current_resitem_installedSize = 0;
	_current_resitem_installOnly = false;
	_current_resitem_packageSet = false;
	_current_resitem_packageUpdateList.clear();

	_current_requires.clear();
	_current_provides.clear();
	_current_conflicts.clear();
	_current_freshens.clear();
	_current_children.clear();
	_current_recommends.clear();
	_current_suggests.clear();
	_current_obsoletes.clear();

    }
    else {
	_DBG("RC_SPEW_XML") << "! Not handling " << name << " at toplevel" << endl;
    }
}


void
XmlParser::resolvableStart(const std::string & name, const xmlChar **attrs)
{
//    _XXX("RC_SPEW_XML") << "XmlParser::resolvableStart(" << name << ")" << endl;

    /* Only care about the containers here */
    if (name == "history") {
	_state = PARSER_HISTORY;
    }
    else if (name == "deps") {
	/*
	 * We can get a <deps> tag surrounding the actual package
	 * dependency sections (requires, provides, conflicts, etc).
	 * In this case, we'll just ignore this tag quietly.
	 */
    }
    else if (name == "requires") {
	_state = PARSER_DEP;
	_current_dep_list = _toplevel_dep_list = &_current_requires;
    }
    else if (name == "recommends") {
	_state = PARSER_DEP;
	_current_dep_list = _toplevel_dep_list = &_current_recommends;
    }
    else if (name == "suggests") {
	_state = PARSER_DEP;
	_current_dep_list = _toplevel_dep_list = &_current_suggests;
    }
    else if (name == "conflicts") {
	bool is_obsolete = false;
	int i;

	_state = PARSER_DEP;

	for (i = 0; attrs && attrs[i] && !is_obsolete; i += 2) {

	    if (!strcasecmp ((const char *)(attrs[i]), "obsoletes"))
		is_obsolete = true;
	}

	if (is_obsolete)
	    _current_dep_list = _toplevel_dep_list = &_current_obsoletes;
	else {
	    _current_dep_list = _toplevel_dep_list = &_current_conflicts;
	}
    }
    else if (name == "obsoletes") {
	_state = PARSER_DEP;
	_current_dep_list = _toplevel_dep_list = &_current_obsoletes;
    }
    else if (name == "provides") {
	_state = PARSER_DEP;
	_current_dep_list = _toplevel_dep_list = &_current_provides;
    }
    else if (name == "children") {
	_state = PARSER_DEP;
	_current_dep_list = _toplevel_dep_list = &_current_children;
    }
    else if (name == "freshens") {
	_state = PARSER_DEP;
	_current_dep_list = _toplevel_dep_list = &_current_freshens;
    }
    else {
//	_XXX("RC_SPEW_XML") << "! Not handling " << name << " in package start" << endl;
    }
}


void
XmlParser::historyStart(const std::string & name, const xmlChar **attrs)
{
//    _XXX("RC_SPEW_XML") << "XmlParser::historyStart(" << name << ")" << endl;

    if (name == "update") {
	assert(_current_update == NULL);

	_current_update = new PackageUpdate(_current_resitem_name, _current_resitem_kind);

	_state = PARSER_UPDATE;
    }
    else {
	_XXX("RC_SPEW_XML") << "! Not handling " << name << " in history" << endl;
    }
}


void
XmlParser::dependencyStart(const std::string & name, const xmlChar **attrs)
{
//    _XXX("RC_SPEW_XML") << "XmlParser::dependencyStart(" << name << ")" << endl;

    if (name == "dep") {
	Capability dep;
	bool is_obsolete;

	dep = parse_dep_attrs(&is_obsolete, attrs);

	if (is_obsolete)
	    _current_obsoletes.insert (dep);
	else {
	    _current_dep_list->insert (dep);
	}
    }
    else if (name == "or")
	_current_dep_list->clear();
    else {
	_DBG("RC_SPEW_XML") <<  "! Not handling " << name << " in dependency" << endl;
    }
}


//---------------------------------------------------------------------------


void
XmlParser::resolvableEnd (const std::string & name)
{
//    _XXX("RC_SPEW_XML") << "XmlParser::resolvableEnd(" << name << ")" << endl;

    if ((name == "package")
	|| (name == "pattern")
	|| (name == "script")
	|| (name == "message")
	|| (name == "patch")
	|| (name == "product")) {

	_current_resitem_kind = string2kind (name);

	PackageUpdate_Ptr update = NULL;
	/* If possible, grab the version info from the most recent update.
	 * Otherwise, try to find where the package provides itself and use
	 * that version info.
	 */
	// searching the highest version. It is not sure anymore that the last
	// has highest version
	if (!_current_resitem_packageUpdateList.empty())
	{
	    for (PackageUpdateList::iterator iter = _current_resitem_packageUpdateList.begin();
		 iter != _current_resitem_packageUpdateList.end();
		 iter++)
	    {
		if (!update)
		    update = *iter;
		else
		{
		    if (update->edition() < (*iter)->edition())
			update = *iter;			
		}
	    }
	}

	if (update) {
	    _current_resitem_name = update->name();
	    _current_resitem_kind = update->kind();
	    _current_resitem_edition = update->edition();
	    _current_resitem_fileSize = update->packageSize();
	    _current_resitem_installedSize = update->installedSize();
	    _current_resitem_arch = update->arch();	 
	}
	else {
	    for (CapSet::const_iterator iter = _current_provides.begin(); iter != _current_provides.end(); iter++) {
		std::string capString = (*iter).asString();
		std::string cmpString = _current_resitem_name + " == ";
		string::size_type ret = capString.find (cmpString);		
		if (ret != string::npos)
		{
		    string editionStr = capString.substr (cmpString.length());
		    _current_resitem_kind = (*iter).refers();
		    _current_resitem_edition = Edition (editionStr);
		    break;
		}
	    }
	}

	// check if we provide ourselfs properly

	CapFactory  factory;
	Capability selfdep = factory.parse ( _current_resitem_kind,
		                           _current_resitem_name,
		                           Rel::EQ,
					 _current_resitem_edition);
	CapSet::const_iterator piter;
	for (piter = _current_provides.begin(); piter != _current_provides.end(); piter++) {
	    if ((*piter) == selfdep)
	    {
		break;
	    }
	}

	if (piter == _current_provides.end()) {			// no self provide found, construct one
	    _XXX("RC_SPEW") << "Adding self-provide [" << selfdep.asString() << "]" << endl;
	    _current_provides.insert (selfdep);
	}

	Package_Ptr package = new Package ( _channel,
					   _current_resitem_kind,
		                           _current_resitem_name,
		                           _current_resitem_edition,
		                           _current_resitem_arch);

	if (_channel->system())				// simulate system channel by loading xml file
	    package->setInstalled (true);

	Dependencies deps;
	deps.requires		= _current_requires;
	deps.provides		= _current_provides;
	deps.conflicts		= _current_conflicts;
	deps.obsoletes		= _current_obsoletes;
	deps.suggests		= _current_suggests;
	deps.recommends		= _current_recommends;
	deps.freshens		= _current_freshens;
	package->deprecatedSetDependencies  (deps);
	package->setPrettyName    (_current_resitem_prettyName);
	package->setSummary       (_current_resitem_summary);
	package->setDescription   (_current_resitem_description);
	package->setFileSize      (_current_resitem_fileSize);
	package->setInstalledSize (_current_resitem_installedSize);
	package->setInstallOnly   (_current_resitem_installOnly);
	package->setPackageSet    (_current_resitem_packageSet);
	for (PackageUpdateList::iterator iter = _current_resitem_packageUpdateList.begin();
	     iter != _current_resitem_packageUpdateList.end();
	     iter++) {
	    PackageUpdate_Ptr update = *iter;
	    update->setPackage (package);
	    package->addUpdate (update);
	}
	_all_packages.push_back (package);

//	_DBG("RC_SPEW") << package->asString(true) << endl;
	_DBG("RC_SPEW_XML") << "XmlParser::resolvableEnd(" << name << ") done: '" << package->asString(true) << "'" << endl;
//	_XXX("RC_SPEW_XML") << "XmlParser::resolvableEnd now " << _all_packages.size() << " packages" << endl;
	_current_resitem_stored = true;
	_state = PARSER_TOPLEVEL;
    }
    else if (name == "name") {			_current_resitem_name = strstrip (_text_buffer);
    } else if (name == "pretty_name") {		_current_resitem_prettyName = strstrip (_text_buffer);
    } else if (name == "summary") {		_current_resitem_summary = strstrip (_text_buffer);
    } else if (name == "description") {		_current_resitem_description = strstrip (_text_buffer);
    } else if (name == "section") {		_current_resitem_section = strstrip (_text_buffer);
    } else if (name == "arch") {		_current_resitem_arch = Arch(strstrip (_text_buffer));
    } else if (name == "filesize") {		_current_resitem_fileSize = atoi(_text_buffer);
    } else if (name == "installedsize") {	_current_resitem_installedSize = atoi(_text_buffer);
    } else if (name == "install_only") {	_current_resitem_installOnly = true;
    } else if (name == "package_set") {		_current_resitem_packageSet = true;
    } else {
	_DBG("RC_SPEW_XML") << "XmlParser::resolvableEnd(" << name << ") unknown" << endl;
    }

//    _XXX("RC_SPEW_XML") << "XmlParser::resolvableEnd(" << name << ") done" << endl;

    releaseBuffer();
}


void
XmlParser::historyEnd (const std::string & name)
{
//    _XXX("RC_SPEW_XML") << "XmlParser::historyEnd(" << name << ")" << endl;

    if (name == "history") {
	assert(_current_update == NULL);

	_state = PARSER_RESOLVABLE;
    }
}


void
XmlParser::updateEnd (const std::string & name)
{
//    _XXX("RC_SPEW_XML") << "XmlParser::updateEnd(" << name << ")" << endl;

    Channel_constPtr channel;
    string url_prefix;

    assert(_current_update != NULL);

    if (_channel != NULL) {
	url_prefix = _channel->filePath ();
    }

    if (name == "update") {
	_current_resitem_packageUpdateList.push_back(_current_update);

	_current_update = NULL;
	_state = PARSER_HISTORY;

    } else if (name == "epoch") {		_current_update->setEpoch (atoi(_text_buffer));
    } else if (name == "version") {		_current_update->setVersion (strstrip (_text_buffer));
    } else if (name == "release") {		_current_update->setRelease (strstrip (_text_buffer));
    } else if (name == "arch") {		_current_update->setArch (strstrip (_text_buffer));
    } else if (name == "filename") {
	strstrip (_text_buffer);
	if (!url_prefix.empty()) {
	    _current_update->setPackageUrl (maybe_merge_paths(url_prefix, _text_buffer));
	}
	else {
	    _current_update->setPackageUrl (_text_buffer);
	}
    } else if (name == "filesize") {		_current_update->setPackageSize (atoi(_text_buffer));
    } else if (name == "installedsize") {	_current_update->setInstalledSize (atoi (_text_buffer));
    } else if (name == "signaturename") {
	strstrip (_text_buffer);
	if (!url_prefix.empty()) {
	    _current_update->setSignatureUrl (maybe_merge_paths(url_prefix, _text_buffer));
	}
	else {
	    _current_update->setSignatureUrl (_text_buffer);
	}
    } else if (name == "signaturesize") {	_current_update->setSignatureSize (atoi (_text_buffer));
    } else if (name == "md5sum") {		_current_update->setMd5sum (strstrip (_text_buffer));
    } else if (name == "importance") {		_current_update->setImportance (Importance::parse (strstrip (_text_buffer)));
    } else if (name == "description") {		_current_update->setDescription (strstrip (_text_buffer));
    } else if (name == "hid") {			_current_update->setHid (atoi(_text_buffer));
    } else if (name == "license") {		_current_update->setLicense (strstrip (_text_buffer));
    } else {
	ERR << "XmlParser::updateEnd(" << name << ") unknown" << endl;
    }

//    if (_current_update != NULL )
//	_DBG("RC_SPEW_XML") << "XmlParser::updateEnd(" << name << ") => '" << _current_update->asString() << "'" << endl;

    releaseBuffer();

}


void
XmlParser::dependencyEnd (const std::string & name)
{
//    _XXX("RC_SPEW_XML") << "XmlParser::dependencyEnd(" << name << ")" << endl;

    if (name == "or") {
#if 0
	OrDependency_Ptr or_dep = OrDependency::fromDependencyList (*_current_dep_list);
	Dependency_Ptr dep = new Dependency (or_dep);

	(*_current_dep_list).clear();

	(*_toplevel_dep_list).push_back (dep);
	_current_dep_list = _toplevel_dep_list;
#endif
    }
    else if (name == "dep") {
	/* We handled everything we needed for dep in start */
    }
    else {
	/* All of the dep lists (requires, provides, etc.) */
	_toplevel_dep_list = NULL;
	_current_dep_list = NULL;
	_state = PARSER_RESOLVABLE;
    }
}



//===================================================================================================================

#if 0
//---------------------------------------------------------------------------

/* ------ */


static RCResItemDep *
rc_xml_node_to_resItem_dep_internal (const xmlNode *node)
{
    gchar *name = NULL, *version = NULL, *release = NULL;
    gboolean has_epoch = false;
    guint32 epoch = 0;
    RCResItemRelation relation;
    RCResItemDep *dep;

    gchar *tmp;

    if (g_strcasecmp (node->name, "dep")) {
	return (NULL);
    }

    name = xml_get_prop (node, "name");
    tmp = xml_get_prop (node, "op");
    if (tmp) {
	relation = rc_resItem_relation_from_string (tmp);

	has_epoch = xml_get_guint32_value (node, "epoch", &epoch);

	version = xml_get_prop (node, "version");
	release = xml_get_prop (node, "release");
    } else {
	relation = RC_RELATION_ANY;
    }

    /* FIXME: should get channel from XML */
    dep = rc_resItem_dep_new (name, has_epoch, epoch, version, release,
		              relation, RC_TYPE_RESOLVABLE, RC_CHANNEL_ANY,
		              false, false);

    g_free (tmp);
    g_free (name);
    g_free (version);
    g_free (release);

    return dep;
} /* rc_xml_node_to_resItem_dep_internal */

RCResItemDep *
rc_xml_node_to_resItem_dep (const xmlNode *node)
{
    RCResItemDep *dep = NULL;

    if (!g_strcasecmp (node->name, "dep")) {
	dep = rc_xml_node_to_resItem_dep_internal (node);
	return (dep);
    } else if (!g_strcasecmp (node->name, "or")) {
	RCResItemDepSList *or_dep_slist = NULL;
	RCDepOr *or;
	xmlNode *iter = node->xmlChildrenNode;

	while (iter) {
	    if (iter->type == XML_ELEMENT_NODE) {
		or_dep_slist = g_slist_append(
		                              or_dep_slist,
		                              rc_xml_node_to_resItem_dep_internal (iter));
	    }

	    iter = iter->next;
	}

	or = rc_dep_or_new (or_dep_slist);
	dep = rc_dep_or_new_provide (or);
    }

    return (dep);
} /* rc_xml_node_to_resItem_dep */

/* ------ */

/* This hack cleans 8-bit characters out of a string.  This is a very
   problematic "solution" to the problem of non-UTF-8 package info. */
static gchar *
sanitize_string (const char *str)
{
    gchar *dup = g_strdup (str);
    gchar *c;

    return dup;

    if (dup) {
	for (c = dup; *c; ++c) {
	    if ((guint)*c > 0x7f)
		*c = '_';
	}
    }

    return dup;
}

#endif
///////////////////////////////////////////////////////////////////
  };// namespace detail
  /////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////
    };// namespace solver
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
