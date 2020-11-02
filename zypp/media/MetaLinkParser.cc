/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MetaLinkParser.cc
 *
 */

#include <zypp/media/MetaLinkParser.h>
#include <zypp/base/Logger.h>
#include <zypp/ByteArray.h>
#include <zypp/AutoDispose.h>

#include <stack>
#include <vector>
#include <algorithm>

#include <libxml2/libxml/SAX2.h>

using namespace zypp::base;

namespace zypp::media {
  enum ParserState {
    STATE_START,
    STATE_METALINK,
    STATE_FILES,
    STATE_FILE,
    STATE_M4FILE,
    STATE_SIZE,
    STATE_M4SIZE,
    STATE_VERIFICATION,
    STATE_HASH,
    STATE_M4HASH,
    STATE_PIECES,
    STATE_M4PIECES,
    STATE_PHASH,
    STATE_M4PHASH,
    STATE_RESOURCES,
    STATE_URL,
    STATE_M4URL,
    NUMSTATES
  };

  struct transition {
    std::string elementName; //< Name of the element for the transition to trigger
    ParserState transitionTo; //< The state we go into when the element name in \a elementName is encountered
    int docontent;   //< Store the content of the element in the \a content member
  };

  /*!
   * Returns a map of all state transitions that are supported.
   * Key of the map is the current state, the value is a list of all supported transitions
   * of the the current state.
   */
  const std::unordered_map<ParserState, std::vector<transition> > & transitions () {
    static std::unordered_map<ParserState, std::vector<transition> > map {
      { STATE_START, {
          { "metalink", STATE_METALINK, 0},
          }
        },
      { STATE_METALINK, {
          { "files", STATE_FILES, 0 },
          { "file", STATE_M4FILE, 0 },
          }
        },
      { STATE_FILES, {
          { "file", STATE_FILE, 0},
          }
        },
      { STATE_FILE, {
          { "size",         STATE_SIZE, 1 },
          { "verification", STATE_VERIFICATION, 0 },
          { "resources",    STATE_RESOURCES, 0 },
          }
        },
      { STATE_VERIFICATION, {
          { "hash",         STATE_HASH, 1 },
          { "pieces",       STATE_PIECES, 0 },
          }
        },
      { STATE_PIECES, {
          { "hash",         STATE_PHASH, 1 },
          }
        },
      { STATE_RESOURCES, {
          { "url",          STATE_URL, 1 },
          }
        },
      { STATE_M4FILE, {
          { "size",         STATE_M4SIZE, 1 },
          { "hash",         STATE_M4HASH, 1},
          { "url",          STATE_M4URL, 1},
          { "pieces",       STATE_M4PIECES, 0},
          }
        },
      { STATE_M4PIECES, {
          { "hash",         STATE_M4PHASH, 1 },
          }
        },
      };

    return map;
  }

static void XMLCALL startElement(void *userData, const xmlChar *name, const xmlChar **atts);
static void XMLCALL endElement(void *userData, const xmlChar *name);
static void XMLCALL characterData(void *userData, const xmlChar *s, int len);

struct ml_parsedata : private zypp::base::NonCopyable {
  ml_parsedata()
    : parser( nullptr )
    , state( STATE_START )
    , depth( 0 )
    , statedepth( 0 )
    , docontent( 0 )
    , gotfile( 0 )
    , size( -1 )
    , blksize( 0 )
    , piecel( 0 )
    , chksuml( 0 )
  {
    content.reserve( 256 );

    xmlSAXHandler sax;
    memset(&sax, 0, sizeof(sax));
    sax.startElement = startElement;
    sax.endElement = endElement;
    sax.characters = characterData;

    //internally creates a copy of xmlSaxHandler, so having it as local variable is save
    parser = AutoDispose<xmlParserCtxtPtr>( xmlCreatePushParserCtxt(&sax, this, NULL, 0, NULL), xmlFreeParserCtxt );
  }

  void doTransition ( const transition &t ) {
    parentStates.push( state );
    state = t.transitionTo;
    docontent = t.docontent;
    statedepth = depth;
    content.clear();
  }

  void popState () {
    state = parentStates.top();
    statedepth--;
    parentStates.pop();

  }

  AutoDispose<xmlParserCtxtPtr> parser;

  ParserState state;  //< current state as defined in \ref stateswitch
  std::stack<ParserState> parentStates;

  int depth;         //< current element depth of traversing the document elements

  /*!
   * Current element depts expected by the current state,
   * if depth != statedepth we ignore all elements and just increase and decrease depth until they match again.
   * This is basically a helper variable that allows us to ignore elements we do not care about
   */
  int statedepth;

  std::string content; //< content of the current element
  int docontent;     //< should the content of the current elem be parsed

  int gotfile;
  off_t size;
  std::vector<MetalinkMirror> urls;
  size_t blksize;

  std::vector<UByteArray> piece;
  int piecel;

  std::vector<UByteArray> sha1;
  std::vector<UByteArray> zsync;

  UByteArray chksum;
  int chksuml;
};

/**
 * Look up a xml attribute in the passed array \a atts.
 * Returns a pointer to the value of the attribute if one is found, otherwise nullptr.
 */
static const char *
find_attr(const char *txt, const xmlChar **atts)
{
  if(!atts) {
    return nullptr;
  }

  for (; *atts; atts += 2)
    {
      if (!strcmp(reinterpret_cast<const char*>(*atts), txt))
        return reinterpret_cast<const char*>(atts[1]);
    }
  return nullptr;
}

static void XMLCALL
startElement(void *userData, const xmlChar *name, const xmlChar **atts)
{
  struct ml_parsedata *pd = reinterpret_cast<struct ml_parsedata *>(userData);

  // if the current element depth does not match the expected depth for the current state we
  // ignore the element and just increase the depth
  if (pd->depth != pd->statedepth) {
    pd->depth++;
    return;
  }
  pd->depth++;

  const auto &trMap = transitions();
  const auto currStateTrs = trMap.find( pd->state );
  if ( currStateTrs == trMap.end() )
    return;

  // check if the current element name is part of our transitions
  auto foundTr = std::find_if( currStateTrs->second.begin(), currStateTrs->second.end(), [name]( const auto &tr ){
    return tr.elementName == reinterpret_cast<const char *>(name);
  });

  if ( foundTr == currStateTrs->second.end() ) {
    // we found no possible transition, ignore
    return;
  }

  if ( ( foundTr->transitionTo == STATE_FILE || foundTr->transitionTo == STATE_M4FILE )  && pd->gotfile++)
    return;	/* ignore all but the first file */

  // advance the state machine and prepare variables for the new state
  pd->doTransition( *foundTr );

  switch(pd->state)
    {
    case STATE_URL:
    case STATE_M4URL:
      {
	const char *priority       = find_attr("priority", atts);
	const char *preference     = find_attr("preference", atts);
        const char *maxconnections = find_attr("maxconnections", atts);
	int prio;
        auto &mirr = pd->urls.emplace_back();
        if (priority)
	  prio = str::strtonum<int>(priority);
	else if (preference)
          prio = 101 - str::strtonum<int>(preference);
	else
	  prio = 999999;
        mirr.priority = prio;

        if ( maxconnections )
          mirr.maxConnections = str::strtonum<int>( maxconnections );

	break;
      }
    case STATE_PIECES:
    case STATE_M4PIECES:
      {
	const char *type = find_attr("type", atts);
	const char *length = find_attr("length", atts);
	size_t blksize;

	if (!type || !length)
	  {
            pd->popState();
	    break;
	  }
	blksize = str::strtonum<unsigned long>(length);
	if (!blksize || (pd->blksize && pd->blksize != blksize))
	  {
	    pd->popState();
	    break;
	  }
	pd->blksize = blksize;
        pd->piece.clear();
	if (!strcmp(type, "sha1") || !strcmp(type, "sha-1"))
	  pd->piecel = 20;
	else if (!strcmp(type, "zsync"))
	  pd->piecel = 4;
	else
	  {
	    pd->popState();
	    break;
	  }
	break;
      }
    case STATE_HASH:
    case STATE_M4HASH:
      {
	const char *type = find_attr("type", atts);
	if (!type)
	  type = "?";
	if ((!strcmp(type, "sha1") || !strcmp(type, "sha-1")) && pd->chksuml < 20)
	  pd->chksuml = 20;
	else if (!strcmp(type, "sha256") || !strcmp(type, "sha-256"))
	  pd->chksuml = 32;
	else
	  {
	    pd->popState();
	    pd->docontent = 0;
	  }
	break;
      }
    case STATE_PHASH:
    case STATE_M4PHASH:
      {
	const char *piece = find_attr("piece", atts);
        if ( pd->state == STATE_PHASH && (!piece || str::strtonum<uint>(piece) != pd->piece.size()) )
	  {
	    pd->popState();
	  }
        break;
      }
    default:
      break;
    }
}

UByteArray hexstr2bytes( std::string str )
{
  return Digest::hexStringToUByteArray( str );
}

static void XMLCALL
endElement(void *userData, const xmlChar *)
{
  struct ml_parsedata *pd = reinterpret_cast<struct ml_parsedata *>(userData);
  //printf("end depth %d-%d name %s\n", pd->depth, pd->statedepth, name);
  if (pd->depth != pd->statedepth)
    {
      pd->depth--;
      return;
    }
  switch (pd->state)
    {
    case STATE_SIZE:
    case STATE_M4SIZE:
      pd->size = (off_t)str::strtonum<off_t>(pd->content); //strtoull(pd->content, 0, 10);
      break;
    case STATE_HASH:
    case STATE_M4HASH:
      pd->chksum.clear();
      pd->chksum = hexstr2bytes( pd->content );
      if ( pd->content.length() != size_t(pd->chksuml) * 2 || !pd->chksum.size() )
	{
	  pd->chksum.clear();
          pd->chksuml = 0;
	}
      break;
    case STATE_PHASH:
    case STATE_M4PHASH: {
      if ( pd->content.length() != size_t(pd->piecel) * 2 )
	break;
      UByteArray pieceHash = hexstr2bytes( pd->content );
      if ( !pieceHash.size() )
        pieceHash.resize( pd->piecel, 0 );
      pd->piece.push_back( pieceHash );
      break;
    }
    case STATE_PIECES:
    case STATE_M4PIECES:
      if (pd->piecel == 4)
	pd->zsync = pd->piece;
      else
        pd->sha1 = pd->piece;

      pd->piecel = 0;
      pd->piece.clear();
      break;
    case STATE_URL:
    case STATE_M4URL:
      if ( pd->content.length() )
        pd->urls.back().url =   std::string(pd->content);
      else
        // without a actual URL the mirror is useless
        pd->urls.pop_back();
      break;
    default:
      break;
    }

  pd->depth--;
  pd->popState();
  pd->docontent = 0;
}

static void XMLCALL
characterData(void *userData, const xmlChar *s, int len)
{
  struct ml_parsedata *pd = reinterpret_cast<struct ml_parsedata *>(userData);
  if (!pd->docontent)
    return;

  if ( pd->content.length() + len + 1 > pd->content.capacity() )
    pd->content.reserve( pd->content.capacity() + 256 );
  pd->content.append( s, s+len );
}


MetaLinkParser::MetaLinkParser()
  : pd( new ml_parsedata )
{}

MetaLinkParser::~MetaLinkParser()
{
  delete pd;
}

void
MetaLinkParser::parse(const Pathname &filename)
{
  parse(InputStream(filename));
}

void
MetaLinkParser::parse(const InputStream &is)
{
  char buf[4096];
  if (!is.stream())
    ZYPP_THROW(Exception("MetaLinkParser: no such file"));
  while (is.stream().good())
    {
      is.stream().read(buf, sizeof(buf));
      parseBytes(buf, is.stream().gcount());
    }
  parseEnd();
}

void
MetaLinkParser::parseBytes(const char *buf, size_t len)
{
  if (!len)
    return;

  if (xmlParseChunk(pd->parser, buf, len, 0)) {
    ZYPP_THROW(Exception("Parse Error"));
  }
}

void
MetaLinkParser::parseEnd()
{
  if (xmlParseChunk(pd->parser, NULL, 0, 1)) {
    ZYPP_THROW(Exception("Parse Error"));
  }
  if (pd->urls.size() ) {
    stable_sort(pd->urls.begin(), pd->urls.end(), []( const auto &a, const auto &b ){
      return a.priority < b.priority;
    });
  }
}

std::vector<Url>
MetaLinkParser::getUrls() const
{
  std::vector<Url> urls;
  for ( const auto &mirr : pd->urls )
    urls.push_back( mirr.url );
  return urls;
}

const std::vector<MetalinkMirror> &MetaLinkParser::getMirrors() const
{
  return pd->urls;
}

MediaBlockList MetaLinkParser::getBlockList() const
{
  MediaBlockList bl(pd->size);
  if (pd->chksuml == 20)
    bl.setFileChecksum("SHA1", pd->chksuml, pd->chksum.data() );
  else if (pd->chksuml == 32)
    bl.setFileChecksum("SHA256", pd->chksuml, pd->chksum.data());
  if (pd->size != off_t(-1) && pd->blksize)
    {
      size_t nb = (pd->size + pd->blksize - 1) / pd->blksize;
      off_t off = 0;
      size_t size = pd->blksize;
      for ( size_t i = 0; i < nb; i++ )
	{
	  if (i == nb - 1)
	    {
	      size = pd->size % pd->blksize;
	      if (!size)
		size = pd->blksize;
	    }
          size_t blkno = bl.addBlock(off, size);
          if ( i < pd->sha1.size())
	    {
	      bl.setChecksum(blkno, "SHA1", 20, pd->sha1[i].data());
              if ( i < pd->zsync.size())
		{
		  unsigned char *p = pd->zsync[i].data();
		  bl.setRsum(blkno, 4, p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24, pd->blksize);
		}
	    }
	  off += pd->blksize;
	}
    }
  return bl;
}

const std::vector<UByteArray> &MetaLinkParser::getZsyncBlockHashes() const
{
  return pd->zsync;
}

const std::vector<UByteArray> &MetaLinkParser::getSHA1BlockHashes() const
{
  return pd->sha1;
}

} // namespace zypp::media
