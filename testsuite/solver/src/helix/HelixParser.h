/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* HelixParser.h: XML routines
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

#ifndef ZYPP_SOLVER_TEMPORARY_HELIXPARSER_H
#define ZYPP_SOLVER_TEMPORARY_HELIXPARSER_H

#include <iosfwd>
#include <string>
#include <list>

#include "zypp/CapSet.h"
#include "zypp/ResStore.h"
#include "zypp/Dependencies.h"
#include "zypp/Source.h"

#include "XmlNode.h"

namespace zypp {

class HelixParser;
class HelixSourceImpl;
typedef bool (*ParserCallback) (const HelixParser & parsed, HelixSourceImpl *impl);
typedef struct {
    std::string name;
    int epoch;
    std::string version;
    std::string release;
    std::string arch;
    long fileSize;
    long installedSize;
} History;
typedef std::list<History> HistoryList;

///////////////////////////////////////////////////////////////////
//
//      CLASS NAME : HelixParser

class HelixParser
{
  public:
    enum _HelixParserState {
	PARSER_TOPLEVEL = 0,
	PARSER_RESOLVABLE,
	PARSER_HISTORY,
	PARSER_UPDATE,
	PARSER_DEP,
    };
    typedef enum _HelixParserState HelixParserState;

  private:
    bool _processing;
    xmlParserCtxtPtr _xml_context;
    HelixParserState _state;

    /* Temporary state */
    bool _stored;


  public:
    Resolvable::Kind kind;
    std::string name;
    int epoch;
    std::string version;
    std::string release;
    std::string arch;
    std::string summary;
    std::string description;
    std::string section;
    int fileSize;
    int installedSize;
    bool installOnly;
    bool installed;
    unsigned int location;

    CapSet provides;
    CapSet prerequires;
    CapSet requires;
    CapSet conflicts;
    CapSet obsoletes;
    CapSet recommends;
    CapSet suggests;
    CapSet freshens;
    CapSet enhances;
    CapSet extends;

  private:
    // these point to one of the Dependency sets during dependency parsing
    CapSet *_dep_set;
    CapSet *_toplevel_dep_set;		// just needed during 'or' parsing

    HistoryList _history;

    std::string _text_buffer;

    void setState (HelixParserState state) { _state = state; }

  public:

    HelixParser ();
    virtual ~HelixParser();

    // ---------------------------------- I/O

    static std::string toString ( const HelixParser & parser);
    virtual std::ostream & dumpOn( std::ostream & str ) const;
    friend std::ostream& operator<<( std::ostream & str, const HelixParser & parser);
    std::string asString ( void ) const;

    // ---------------------------------- accessors

    bool processing (void) const { return _processing; }
    void setProcessing (bool processing) { _processing = processing; }

  public:
    // ---------------------------------- accessors

    HelixParserState state (void) const { return _state; }

    // ---------------------------------- methods
    // callbacks from xml_ parser functions

    void toBuffer (std::string data);
    void releaseBuffer (void);		// free _text_buffer

    void startElement(const std::string & token, const xmlChar **attrs);
    void endElement(const std::string & token);

    void parseChunk (const char *xmlbuf, size_t size, HelixSourceImpl *impl);
    void done (void);

  private:
    HelixSourceImpl *_impl;

    // internal HelixParser functions, c++-style
    void toplevelStart(const std::string & token, const xmlChar **attrs);
    void resolvableStart(const std::string & token, const xmlChar **attrs);
    void historyStart(const std::string & token, const xmlChar **attrs);
    void dependencyStart(const std::string & token, const xmlChar **attrs);

    void updateEnd(const std::string & token);
    void resolvableEnd(const std::string & token);
    void historyEnd(const std::string & token);
    void dependencyEnd(const std::string & token);

};

} // namespace zypp

#endif  // ZYPP_SOLVER_TEMPORARY_HELIXPARSER_H
