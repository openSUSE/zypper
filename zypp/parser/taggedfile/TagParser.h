/*---------------------------------------------------------------------\
 |                                                                      |
 |                      __   __    ____ _____ ____                      |
 |                      \ \ / /_ _/ ___|_   _|___ \                     |
 |                       \ V / _` \___ \ | |   __) |                    |
 |                        | | (_| |___) || |  / __/                     |
 |                        |_|\__,_|____/ |_| |_____|                    |
 |                                                                      |
 |                               core system                            |
 |                                                        (C) SuSE GmbH |
 \----------------------------------------------------------------------/

 File:       TagParser.h

 Author:     Michael Andres <ma@suse.de>
 Maintainer: Michael Andres <ma@suse.de>

 /-*/
#ifndef TagParser_h
#define TagParser_h

#include <iosfwd>
#include <string>
#include <list>
#include <vector>
#include <map>

using std::vector;
using std::map;
using std::streamoff;
using std::istream;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TagParser
//
//
//	DESCRIPTION :
//
class TagParser {

  private:

    static const unsigned bufferLen_i;
    static char           buffer_ac[];

  private:

    streamoff lookupStart_i;
    streamoff startTag_i;
    streamoff startData_i;
    streamoff endData_i;
    streamoff endTag_i;

    std::string    startTag_t;
    std::string    endTag_t;

    std::list<std::string> data_Vt;

  private:

    void _reset();
    void _datareset();

    static streamoff  readLine( istream & stream_fr, std::string & cline_tr );
    static bool       tagOnLine( const std::string & cline_tr, std::string & tag_tr, std::string::size_type & delim_ir );

  public:

    TagParser();
    virtual ~TagParser();

    static const streamoff nopos;

    streamoff      lookupStart() const { return lookupStart_i; }

    streamoff      posStartTag() const { return startTag_i; }
    const std::string & startTag()    const { return startTag_t; }

    streamoff      posEndTag()   const { return endTag_i; }
    const std::string & endTag()      const { return endTag_t; }

  public:

    streamoff      posDataStart()const { return startData_i; }
    streamoff      posDataEnd()  const { return endData_i; }
    unsigned       dataLength()  const { return endData_i - startData_i; }
    unsigned       dataLines()   const { return data_Vt.size(); }

    const std::list<std::string> & data() const { return data_Vt; }

  public:

    // parse until stag_tr, leave empty to parse until any tag. On succes
    // posStartTag() and posEndTag() can be used
    bool lookupTag( istream & stream_fr, const std::string & stag_tr = "" );
    // continue parsing until end tag etag_tr is found since the above
    // function sets posEndTag to the end of the line
    bool parseData( istream & stream_fr, const std::string & etag_tr );

    static bool retrieveData( istream & stream_fr,
			      streamoff startData_ir, streamoff endData_ir,
			      std::string & data_tr );
    static bool retrieveData( istream & stream_fr,
			      streamoff startData_ir, streamoff endData_ir,
			      std::list<std::string> & data_Vtr );
    static std::string data2string( const std::list<std::string> & data_Vtr );
    static vector<std::string> split2words( const std::string & line_tr, const std::string & sepchars_tr = " \t\n" );
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TagSet
//
//	DESCRIPTION :
//
#if 0
class TagSet {

  friend std::ostream & operator<<( std::ostream & str, const TagSet & obj );

  private:

    map<std::string,unsigned> lookupStart_V;
    vector<std::string>       lookupEnd_V;

  public:

    static const unsigned unknowntag = (unsigned)-1;

    TagSet() {}
    virtual ~TagSet() {}

    void addTag( const std::string & start_tr, const std::string & end_tr = "" );

    unsigned parseData( istream & in_Fr, TagParser & ctag_Cr ) const;
};

///////////////////////////////////////////////////////////////////

extern std::ostream & operator<<( std::ostream & str, const TagParser & obj );
extern std::ostream & operator<<( std::ostream & str, const TagSet & obj );

///////////////////////////////////////////////////////////////////
#endif		// #if 0

#endif // TagParser_h

// vim:sw=2
