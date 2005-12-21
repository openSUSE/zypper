/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <config.h>
#include <ctype.h>
#include <assert.h>
#include <zypp/solver/detail/XmlParser.h>
#include <zypp/solver/detail/Dependency.h>
#include <zypp/solver/detail/OrDependency.h>
#include <zypp/solver/detail/PackageUpdate.h>
#include <zypp/solver/detail/utils.h>

#include <zypp/solver/detail/debug.h>
#include <zypp/ResObject.h>

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
      
      static DependencyPtr
      parse_dep_attrs(bool *is_obsolete, const xmlChar **attrs)
      {
          int i;
          bool op_present = false;
          /* Temporary variables dependent upon the presense of an 'op' attribute */
          const char *name = NULL;
          int epoch = Edition::noepoch;
          string version;
          string release;
          string arch = "";
          Relation relation = Relation::Any;
      
          *is_obsolete = false;
      
          for (i = 0; attrs[i]; i++) {
      	const char *attr = (const char *)attrs[i++];
      	const char *value = (const char *)attrs[i];
      
      	if (!strcasecmp(attr, "name"))		    name = value;
      	else if (!strcasecmp(attr, "op")) {	    op_present = true; relation = Relation::parse(value); }
      	else if (!strcasecmp(attr, "epoch"))	    epoch = atoi (value);
      	else if (!strcasecmp(attr, "version"))      version = value;
      	else if (!strcasecmp(attr, "release"))	    release = value;
      	else if (!strcasecmp(attr, "arch"))	    arch = value;
      	else if (!strcasecmp (attr, "obsoletes"))   *is_obsolete = true;
      	else {
      	    if (getenv ("RC_SPEW_XML"))	rc_debug (RC_DEBUG_LEVEL_ALWAYS, "! Unknown attribute: %s = %s", attr, value);
      	}
      
          }
      
          /* FIXME: should get Channel from XML */
          /* FIXME: should get Kind from XML */
          if ( std::strlen(arch.c_str()) > 0)
          {
              return new Dependency (name, relation, ResTraits<zypp::Package>::kind, new Channel(CHANNEL_TYPE_ANY), epoch, version, release, zypp::Arch(arch));
          }
          else
          {
              return new Dependency (name, relation, ResTraits<zypp::Package>::kind, new Channel(CHANNEL_TYPE_ANY), epoch, version, release, zypp::Arch_noarch);
          }              
      }
      
      
      //---------------------------------------------------------------------------
      // SAX callbacks
      
      static void
      sax_start_document(void *ptr)
      {
          XmlParser *ctx = (XmlParser *)ptr;
          if (ctx->processing()) return;
      
      //    if (getenv ("RC_SPEW_XML")) rc_debug (RC_DEBUG_LEVEL_ALWAYS, "* Start document");
      
          ctx->setProcessing (true);
      }
      
      
      static void
      sax_end_document(void *ptr)
      {
          XmlParser *ctx = (XmlParser *)ptr;
          if (!ctx->processing()) return;
      
      //    if (getenv ("RC_SPEW_XML")) rc_debug (RC_DEBUG_LEVEL_ALWAYS, "* End document");
      
          ctx->setProcessing (false);
      }
      
      
      static void
      sax_start_element(void *ptr, const xmlChar *name, const xmlChar **attrs)
      {
          XmlParser *ctx = (XmlParser *)ptr;
      
          ctx->releaseBuffer();
      
      #if 0
      //    if (getenv ("RC_SPEW_XML"))	rc_debug (RC_DEBUG_LEVEL_ALWAYS, "* Start element (%s)", (const char *)name);
      
          if (attrs) {
      	for (int i = 0; attrs[i]; i += 2) {
      	    if (getenv ("RC_SPEW_XML")) rc_debug (RC_DEBUG_LEVEL_ALWAYS, "   - Attribute (%s=%s)", (const char *)attrs[i], (const char *)attrs[i+1]);
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
      
      //    if (getenv ("RC_SPEW_XML"))	rc_debug (RC_DEBUG_LEVEL_ALWAYS, "* End element (%s)", (const char *)name);
          
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
      
          if (vsnprintf(tmp, 2048, msg, args) >= 2048) fprintf (stderr, "vsnprintf overflow\n");
          rc_debug (RC_DEBUG_LEVEL_WARNING, "* SAX Warning: %s", tmp);
      
          va_end(args);
      }
      
      
      static void
      sax_error(void *ptr, const char *msg, ...)
      {
          va_list args;
          char tmp[2048];
      
          va_start(args, msg);
      
          if (vsnprintf(tmp, 2048, msg, args) >= 2048) fprintf (stderr, "vsnprintf overflow\n");
          rc_debug (RC_DEBUG_LEVEL_ERROR, "* SAX Error: %s", tmp);
      
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
      
      XmlParser::XmlParser (constChannelPtr channel)
          : _channel (channel)
          , _processing (false)
          , _xml_context (NULL)
          , _state (PARSER_TOPLEVEL)
          , _current_package (NULL)
          , _current_update (NULL)
          , _toplevel_dep_list (NULL)
          , _current_dep_list (NULL)
          , _text_buffer (NULL)
          , _text_buffer_size (0)
      {
      //    if (getenv ("RC_SPEW_XML")) rc_debug (RC_DEBUG_LEVEL_ALWAYS, "* Context created (%p)", this);
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
      
      //    if (getenv ("RC_SPEW_XML"))	fprintf (stderr, "XmlParser[%p]::toBuffer(%.32s...,%ld)\n", this, data, (long)size);
      }
      
      
      void
      XmlParser::releaseBuffer ()
      {
          if (_text_buffer)
      	free (_text_buffer);
          _text_buffer = NULL;
          _text_buffer_size = 0;
      //    if (getenv ("RC_SPEW_XML"))	fprintf (stderr, "XmlParser[%p]::releaseBuffer()\n", this);
      }
      
      
      void
      XmlParser::parseChunk(const char *xmlbuf, size_t size)
      {
          if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::parseChunk(%.32s...,%ld)\n", xmlbuf, (long)size);
      
          xmlSubstituteEntitiesDefault(true);
          
          if (!_xml_context) {
      	_xml_context = xmlCreatePushParserCtxt(&sax_handler, this, NULL, 0, NULL);
          }
      
          xmlParseChunk(_xml_context, xmlbuf, size, 0);
      }
      
      
      PackageList
      XmlParser::done()
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::done()\n");
      
          if (_processing)
      	xmlParseChunk(_xml_context, NULL, 0, 1);
      
          if (_xml_context)
      	xmlFreeParserCtxt(_xml_context);
          
          if (_current_package) {
      	fprintf (stderr, "Incomplete package lost\n");
          }
      
          if (_current_update) {
      	fprintf (stderr, "Incomplete update lost");
          }
      
          return _all_packages;
      }
      
      
      //---------------------------------------------------------------------------
      // Parser state callbacks
      
      void
      XmlParser::startElement(const char *name, const xmlChar **attrs)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::startElement(%s)\n", name);
      
          switch (_state) {
          case PARSER_TOPLEVEL:
      	toplevelStart(name, attrs);
      	break;
          case PARSER_PACKAGE:
      	packageStart(name, attrs);
      	break;
          case PARSER_HISTORY:
      	historyStart(name, attrs);
      	break;
          case PARSER_DEP:
      	dependencyStart(name, attrs);
      	break;
          default:
      	break;
          }
      
          return;
      }
      
      
      void
      XmlParser::endElement(const char *name)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::endElement(%s)\n", name);
      
          if (name != NULL) {			// sax_end_element might set name to NULL
      	switch (_state) {
      	    case PARSER_PACKAGE:
      		packageEnd(name);
      		break;
      	    case PARSER_HISTORY:
      		historyEnd(name);
      		break;
      	    case PARSER_UPDATE:
      		updateEnd(name);
      		break;
      	    case PARSER_DEP:
      		dependencyEnd(name);
      		break;
      	    default:
      		break;
      	}
          }
      
          releaseBuffer();
      
          return;
      }
      
      
      void
      XmlParser::toplevelStart(const char * name, const xmlChar **attrs)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::toplevelStart(%s)\n", name);
      
          if (!strcmp(name, "package")) {
      	assert(_current_package == NULL);
      
      	_state = PARSER_PACKAGE;
      
      	_current_package = new Package(_channel);
      	_current_requires.clear();
      	_current_provides.clear();
      	_current_conflicts.clear();
      	_current_children.clear();
      	_current_recommends.clear();
      	_current_suggests.clear();
      	_current_obsoletes.clear();
      
          }
          else {
      	if (getenv ("RC_SPEW_XML")) rc_debug (RC_DEBUG_LEVEL_ALWAYS, "! Not handling %s at toplevel", (const char *)name);
          }
      }
      
      
      void
      XmlParser::packageStart(const char * name, const xmlChar **attrs)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::packageStart(%s)\n", name);
      
          assert(_current_package != NULL);
      
          /* Only care about the containers here */
          if (!strcmp((const char *)name, "history")) {
      	_state = PARSER_HISTORY;
          }
          else if (!strcmp (name, "deps")) {
      	/*
      	 * We can get a <deps> tag surrounding the actual package
      	 * dependency sections (requires, provides, conflicts, etc).
      	 * In this case, we'll just ignore this tag quietly.
      	 */
          }
          else if (!strcmp(name, "requires")) {
      	_state = PARSER_DEP;
      	_current_dep_list = _toplevel_dep_list = &_current_requires;
          }
          else if (!strcmp(name, "recommends")) {
      	_state = PARSER_DEP;
      	_current_dep_list = _toplevel_dep_list = &_current_recommends;
          }
          else if (!strcmp(name, "suggests")) {
      	_state = PARSER_DEP;
      	_current_dep_list = _toplevel_dep_list = &_current_suggests;
          }
          else if (!strcmp(name, "conflicts")) {
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
          else if (!strcmp(name, "obsoletes")) {
      	_state = PARSER_DEP;
      	_current_dep_list = _toplevel_dep_list = &_current_obsoletes;
          }
          else if (!strcmp(name, "provides")) {
      	_state = PARSER_DEP;
      	_current_dep_list = _toplevel_dep_list = &_current_provides;
          }
          else if (!strcmp(name, "children")) {
      	_state = PARSER_DEP;
      	_current_dep_list = _toplevel_dep_list = &_current_children;
          } 
          else {
      //	if (getenv ("RC_SPEW_XML")) rc_debug (RC_DEBUG_LEVEL_ALWAYS, "! Not handling %s in package start", name);
          }
      }
      
      
      void
      XmlParser::historyStart(const char * name, const xmlChar **attrs)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::historyStart(%s)\n", name);
      
          assert(_current_package != NULL);
      
          if (!strcmp(name, "update")) {
      	assert(_current_update == NULL);
      
      	_current_update = new PackageUpdate(_current_package->name());
      
      	_state = PARSER_UPDATE;
          }
          else {
      	if (getenv ("RC_SPEW_XML")) rc_debug (RC_DEBUG_LEVEL_ALWAYS, "! Not handling %s in history", name);
          }
      }
      
      
      void
      XmlParser::dependencyStart(const char *name, const xmlChar **attrs)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::dependencyStart(%s)\n", name);
      
          if (!strcmp(name, "dep")) {
      	DependencyPtr dep;
      	bool is_obsolete;
      
      	dep = parse_dep_attrs(&is_obsolete, attrs);
      
      	if (is_obsolete)
      	    _current_obsoletes.push_back (dep);
      	else {
      	    _current_dep_list->push_back (dep);
      	}
          }
          else if (!strcmp(name, "or"))
      	_current_dep_list = new CDependencyList;
          else {
      	if (getenv ("RC_SPEW_XML")) rc_debug (RC_DEBUG_LEVEL_ALWAYS, "! Not handling %s in dependency", name);
          }
      }
      
      
      //---------------------------------------------------------------------------
      
      
      void
      XmlParser::packageEnd(const char *name)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::packageEnd(%s)\n", name);
      
          assert(_current_package != NULL);
      
          if (!strcmp(name, "package")) {
      	PackageUpdatePtr update;
      
      	/* If possible, grab the version info from the most recent update.
      	 * Otherwise, try to find where the package provides itself and use
      	 * that version info.
      	 */
      	update = _current_package->getLatestUpdate();
      
      	if (update) {
      	    _current_package->setName (update->name());
      	    _current_package->setKind (update->kind());
      	    _current_package->setEdition (update->edition());
      	    _current_package->setFileSize (update->packageSize());
      	    _current_package->setInstalledSize (update->installedSize());
      	}
      	else {
      	    for (CDependencyList::const_iterator iter = _current_provides.begin(); iter != _current_provides.end(); iter++) {
      		if ((*iter)->relation().isEqual()
      		    && ((*iter)->name() == _current_package->name()))
      		{
      		    _current_package->setKind ((*iter)->kind());
      		    _current_package->setEdition ((*iter)->edition());
      		    break;
      		}
      	    }
      	}

#if 0
      	/* Hack for the old XML */
      	if (_current_package->arch()->isUnknown()) {
      	    _current_package->setArch (zypp::Arch::System);
      	}
#endif

        
      	// check if we provide ourselfs properly
      
      	CDependencyList::const_iterator piter;
      	for (piter = _current_provides.begin(); piter != _current_provides.end(); piter++) {
      	    if ((*piter)->relation().isEqual()
      		&& ((*piter)->name() == _current_package->name()))
      	    {
      		break;
      	    }
      	}
      
      	if (piter == _current_provides.end()) {			// no self provide found, construct one
      	    constDependencyPtr selfdep = new Dependency (_current_package->name(), Relation::Equal, _current_package->kind(), _current_package->channel(), _current_package->edition());
      //if (getenv ("RC_SPEW")) fprintf (stderr, "Adding self-provide [%s]\n", selfdep->asString().c_str());
      	    _current_provides.push_front (selfdep);
      	}
      
      	_current_package->setRequires   (_current_requires);
      	_current_package->setProvides   (_current_provides);
      	_current_package->setConflicts  (_current_conflicts);
      	_current_package->setObsoletes  (_current_obsoletes);
      	_current_package->setSuggests   (_current_suggests);
      	_current_package->setRecommends (_current_recommends);
      
      	_all_packages.push_back (_current_package);
      	
      	if (getenv ("RC_SPEW")) fprintf (stderr, "%s\n", _current_package->asString(true).c_str());
      	if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::packageEnd done: '%s'\n", _current_package->asString(true).c_str());
      //	if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::packageEnd now %ld packages\n", _all_packages.size());
      	_current_package = NULL;
      	_state = PARSER_TOPLEVEL;
          }
          else if (!strcmp(name, "name")) {			_current_package->setName (strstrip (_text_buffer));
          } else if (!strcmp(name, "pretty_name")) {		_current_package->setPrettyName (strstrip (_text_buffer));
          } else if (!strcmp(name, "summary")) {		_current_package->setSummary (strstrip (_text_buffer));
          } else if (!strcmp(name, "description")) {		_current_package->setDescription (strstrip (_text_buffer));
          } else if (!strcmp(name, "section")) {		_current_package->setSection (new Section(strstrip (_text_buffer)));
          } else if (!strcmp(name, "arch")) {			_current_package->setArch (strstrip (_text_buffer));
          } else if (!strcmp(name, "filesize")) {		_current_package->setFileSize (atoi(_text_buffer));
          } else if (!strcmp(name, "installedsize")) {	_current_package->setInstalledSize (atoi(_text_buffer));
          } else if (!strcmp(name, "install_only")) {		_current_package->setInstallOnly (true);
          } else if (!strcmp(name, "package_set")) {		_current_package->setPackageSet (true);
          } else {
      	if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::packageEnd(%s) unknown\n", name);
          }
      
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::packageEnd(%s) done\n", name);
      
          releaseBuffer();
      }
      
      
      void
      XmlParser::historyEnd(const char *name)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::historyEnd(%s)\n", name);
          assert(_current_package != NULL);
      
          if (!strcmp(name, "history")) {
      	assert(_current_update == NULL);
      
      	_state = PARSER_PACKAGE;
          }
      }
      
      
      void
      XmlParser::updateEnd(const char *name)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::updateEnd(%s)\n", name);
      
          constChannelPtr channel;
          const char *url_prefix = NULL;
      
          assert(_current_package != NULL);
          assert(_current_update != NULL);
      
          channel = _current_package->channel();
      
          if (channel != NULL) {
      	url_prefix = channel->filePath ();
          }
      
          if (!strcmp(name, "update")) {
      	_current_package->addUpdate(_current_update);
      
      	_current_update = NULL;
      	_state = PARSER_HISTORY;
      
          } else if (!strcmp(name, "epoch")) {		_current_update->setEpoch (atoi(_text_buffer));
          } else if (!strcmp(name, "version")) {		_current_update->setVersion (strstrip (_text_buffer));
          } else if (!strcmp(name, "release")) {		_current_update->setRelease (strstrip (_text_buffer));
          } else if (!strcmp(name, "arch")) {			_current_update->setArch (strstrip (_text_buffer));
          } else if (!strcmp(name, "filename")) {
      	strstrip (_text_buffer);
      	if (url_prefix) {
      	    _current_update->setPackageUrl (maybe_merge_paths(url_prefix, _text_buffer));
      	}
      	else {
      	    _current_update->setPackageUrl (_text_buffer);
      	}
          } else if (!strcmp(name, "filesize")) {		_current_update->setPackageSize (atoi(_text_buffer));
          } else if (!strcmp(name, "installedsize")) {	_current_update->setInstalledSize (atoi (_text_buffer));
          } else if (!strcmp(name, "signaturename")) {
      	strstrip (_text_buffer);
      	if (url_prefix) {
      	    _current_update->setSignatureUrl (maybe_merge_paths(url_prefix, _text_buffer));
      	}
      	else {
      	    _current_update->setSignatureUrl (_text_buffer);
      	}
          } else if (!strcmp(name, "signaturesize")) {	_current_update->setSignatureSize (atoi (_text_buffer));
          } else if (!strcmp(name, "md5sum")) {		_current_update->setMd5sum (strstrip (_text_buffer));
          } else if (!strcmp(name, "importance")) {		_current_update->setImportance (new Importance (strstrip (_text_buffer)));
          } else if (!strcmp(name, "description")) {		_current_update->setDescription (strstrip (_text_buffer));
          } else if (!strcmp(name, "hid")) {			_current_update->setHid (atoi(_text_buffer));
          } else if (!strcmp (name, "license")) {		_current_update->setLicense (strstrip (_text_buffer));
          } else {
      	fprintf (stderr, "XmlParser::updateEnd(%s) unknown\n", name);
          }
      
      //    if (_current_update != NULL && getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::updateEnd(%s) => '%s'\n", name, _current_update->asString().c_str());
      
          releaseBuffer();
      
      }
      
      
      void
      XmlParser::dependencyEnd(const char *name)
      {
      //    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "XmlParser::dependencyEnd(%s)\n", name);
      
          if (!strcmp(name, "or")) {
      	OrDependencyPtr or_dep = OrDependency::fromDependencyList (*_current_dep_list);
      	DependencyPtr dep = new Dependency (or_dep);
      
      	(*_current_dep_list).clear();
      
      	(*_toplevel_dep_list).push_back (dep);
      	_current_dep_list = _toplevel_dep_list;
          }
          else if (!strcmp(name, "dep")) {
      	/* We handled everything we needed for dep in start */
          }
          else {
      	/* All of the dep lists (requires, provides, etc.) */
      	_toplevel_dep_list = NULL;
      	_current_dep_list = NULL;
      	_state = PARSER_PACKAGE;
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
