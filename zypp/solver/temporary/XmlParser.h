/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* XmlParser.h: XML routines
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

#ifndef ZYPP_SOLVER_TEMPORARY_XMLPARSER_H
#define ZYPP_SOLVER_TEMPORARY_XMLPARSER_H

#include <list>

#include <iosfwd>
#include <string>

#include "zypp/CapSet.h"

#include "zypp/solver/temporary/Channel.h"
#include "zypp/solver/temporary/Package.h"
#include "zypp/solver/temporary/XmlNode.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////


      ///////////////////////////////////////////////////////////////////
      //
      //      CLASS NAME : XmlParser

      class XmlParser
      {
        public:
          enum _XmlParserState {
      	PARSER_TOPLEVEL = 0,
      	PARSER_RESOLVABLE,
      	PARSER_HISTORY,
      	PARSER_UPDATE,
      	PARSER_DEP,
          };
          typedef enum _XmlParserState XmlParserState;

        private:
          Channel_constPtr _channel;
          bool _processing;
          xmlParserCtxtPtr _xml_context;
          XmlParserState _state;

          PackageList _all_packages;

          /* Temporary state */
          bool _current_resitem_stored;
          std::string _current_resitem_name;
          std::string _current_resitem_prettyName;
          std::string _current_resitem_summary;
          std::string _current_resitem_description;
          std::string _current_resitem_section;
          Arch _current_resitem_arch;
          Resolvable::Kind _current_resitem_kind;
          Edition _current_resitem_edition;
          int _current_resitem_fileSize;
          int _current_resitem_installedSize;
          bool _current_resitem_installOnly;
          bool _current_resitem_packageSet;
          PackageUpdateList _current_resitem_packageUpdateList;

          CapSet _current_requires;
          CapSet _current_provides;
          CapSet _current_conflicts;
          CapSet _current_children;
          CapSet _current_recommends;
          CapSet _current_suggests;
          CapSet _current_obsoletes;
          CapSet _current_freshens;
          PackageUpdate_Ptr _current_update;

          // these point to one of the above lists during dependency parsing
          CapSet *_toplevel_dep_list;
          CapSet *_current_dep_list;

          char *_text_buffer;
          size_t _text_buffer_size;

        protected:
          void setState (XmlParserState state) { _state = state; }

        public:

          XmlParser (Channel_constPtr channel);
          virtual ~XmlParser();

          // ---------------------------------- I/O

          static std::string toString ( const XmlParser & parser);

          virtual std::ostream & dumpOn( std::ostream & str ) const;

          friend std::ostream& operator<<( std::ostream & str, const XmlParser & parser);

          std::string asString ( void ) const;

          // ---------------------------------- accessors

          bool processing() const { return _processing; }
          void setProcessing (bool processing) { _processing = processing; }

          XmlParserState state (void) const { return _state; }

      #if 0	// are they needed ?
          Channel_constPtr channel() const { return _channel; }
          void setChannel (Channel_constPtr channel) { _channel = channel; }

          xmlParserCtxtPtr xmlContext() const { return _xml_context; }
          void setXmlContext (xmlParserCtxtPtr xml_context) { _xml_context = xml_context; }
      #endif
          // ---------------------------------- methods

          void toBuffer (const char *data, size_t size);
          void releaseBuffer (void);		// free _text_buffer

	  // callbacks for XML parser, c-style
          void startElement(const char *name, const xmlChar **attrs);
          void endElement(const char *name);

	  // internal XmlParser functions, c++-style
          void toplevelStart(const std::string & name, const xmlChar **attrs);
          void resolvableStart(const std::string & name, const xmlChar **attrs);
          void historyStart(const std::string & name, const xmlChar **attrs);
          void dependencyStart(const std::string & name, const xmlChar **attrs);

          void updateEnd(const std::string & name);
          void resolvableEnd(const std::string & name);
          void historyEnd(const std::string & name);
          void dependencyEnd(const std::string & name);

          void parseChunk (const char *xmlbuf, size_t size);
          PackageList done (void);
      };

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////


#endif  // ZYPP_SOLVER_TEMPORARY_XMLPARSER_H
