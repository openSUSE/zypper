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

#include "zypp/media/MetaLinkParser.h"
#include "zypp/base/Logger.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;
using namespace zypp::base;

namespace zypp {
  namespace media {

enum state {
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

struct stateswitch {
  enum state from;
  string ename;
  enum state to;
  int docontent;
};

static struct stateswitch stateswitches[] = {
  { STATE_START,        "metalink",     STATE_METALINK, 0 },
  { STATE_METALINK,     "files",        STATE_FILES, 0 },
  { STATE_METALINK,     "file",         STATE_M4FILE, 0 },
  { STATE_FILES,        "file",         STATE_FILE, 0 },
  { STATE_FILE,         "size",         STATE_SIZE, 1 },
  { STATE_FILE,         "verification", STATE_VERIFICATION, 0 },
  { STATE_FILE,         "resources",    STATE_RESOURCES, 0 },
  { STATE_VERIFICATION, "hash",         STATE_HASH, 1 },
  { STATE_VERIFICATION, "pieces",       STATE_PIECES, 0 },
  { STATE_PIECES,       "hash",         STATE_PHASH, 1 },
  { STATE_RESOURCES,    "url",          STATE_URL, 1 },
  { STATE_M4FILE,       "size",         STATE_M4SIZE, 1 },
  { STATE_M4FILE,       "hash",         STATE_M4HASH, 1},
  { STATE_M4FILE,       "url",          STATE_M4URL, 1},
  { STATE_M4FILE,       "pieces",       STATE_M4PIECES, 0},
  { STATE_M4PIECES,     "hash",         STATE_M4PHASH, 1 },
  { NUMSTATES }
};

struct ml_url {
  int priority;
  string url;
};

struct ml_parsedata {
  XML_Parser parser;
  int depth;
  enum state state;
  int statedepth;
  char *content;
  int lcontent;
  int acontent;
  int docontent;
  struct stateswitch *swtab[NUMSTATES];
  enum state sbtab[NUMSTATES];

  int called;
  int gotfile;
  off_t size;
  vector<struct ml_url> urls;
  int nurls;
  size_t blksize;

  vector<unsigned char> piece;
  int npiece;
  int piecel;

  vector<unsigned char> sha1;
  int nsha1;
  vector<unsigned char> zsync;
  int nzsync;

  vector<unsigned char> chksum;
  int chksuml;
};

static const char *
find_attr(const char *txt, const char **atts)
{
  for (; *atts; atts += 2)
    {
      if (!strcmp(*atts, txt))
        return atts[1];
    }
  return 0;
}

static void XMLCALL
startElement(void *userData, const char *name, const char **atts)
{
  struct ml_parsedata *pd = reinterpret_cast<struct ml_parsedata *>(userData);
  struct stateswitch *sw;
  if (pd->depth != pd->statedepth)
    {
      pd->depth++;
      return;
    }
  pd->depth++;
  if (!pd->swtab[pd->state])
    return;
  for (sw = pd->swtab[pd->state]; sw->from == pd->state; sw++)  /* find name in statetable */
    if (sw->ename == name)
      break;
  if (sw->from != pd->state)
    return;
  if ((sw->to == STATE_FILE || sw->to == STATE_M4FILE) && pd->gotfile++)
    return;	/* ignore all but the first file */
  //printf("start depth %d name %s\n", pd->depth, name);
  pd->state = sw->to;
  pd->docontent = sw->docontent;
  pd->statedepth = pd->depth;
  pd->lcontent = 0;
  *pd->content = 0;
  switch(pd->state)
    {
    case STATE_URL:
    case STATE_M4URL:
      {
	const char *priority = find_attr("priority", atts);
	const char *preference = find_attr("preference", atts);
	int prio;
        pd->urls.push_back(ml_url());
        if (priority)
	  prio = atoi(priority);
	else if (preference)
	  prio = 101 - atoi(preference);
	else
	  prio = 999999;
	pd->urls.back().priority = prio;
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
	    pd->state = pd->sbtab[pd->state];
	    pd->statedepth--;
	    break;
	  }
	blksize = strtoul(length, 0, 10);
	if (!blksize || (pd->blksize && pd->blksize != blksize))
	  {
	    pd->state = pd->sbtab[pd->state];
	    pd->statedepth--;
	    break;
	  }
	pd->blksize = blksize;
	pd->npiece = 0;
        pd->piece.clear();
	if (!strcmp(type, "sha1") || !strcmp(type, "sha-1"))
	  pd->piecel = 20;
	else if (!strcmp(type, "zsync"))
	  pd->piecel = 4;
	else
	  {
	    pd->state = pd->sbtab[pd->state];
	    pd->statedepth--;
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
	    pd->state = pd->sbtab[pd->state];
	    pd->statedepth--;
	    pd->docontent = 0;
	  }
	break;
      }
    case STATE_PHASH:
    case STATE_M4PHASH:
      {
	const char *piece = find_attr("piece", atts);
	if (pd->state == STATE_PHASH && (!piece || atoi(piece) != pd->npiece))
	  {
	    pd->state = pd->sbtab[pd->state];
	    pd->statedepth--;
	  }
        break;
      }
    default:
      break;
    }
}

static int
hexstr2bytes(unsigned char *buf, const char *str, int buflen)
{
  int i;
  for (i = 0; i < buflen; i++)
    {
#define c2h(c) (((c)>='0' && (c)<='9') ? ((c)-'0')              \
                : ((c)>='a' && (c)<='f') ? ((c)-('a'-10))       \
                : ((c)>='A' && (c)<='F') ? ((c)-('A'-10))       \
                : -1)
      int v = c2h(*str);
      str++;
      if (v < 0)
        return 0;
      buf[i] = v;
      v = c2h(*str);
      str++;
      if (v < 0)
        return 0;
      buf[i] = (buf[i] << 4) | v;
#undef c2h
    }
  return buflen;
}

static void XMLCALL
endElement(void *userData, const char *name)
{
  struct ml_parsedata *pd = reinterpret_cast<struct ml_parsedata *>(userData);
  // printf("end depth %d-%d name %s\n", pd->depth, pd->statedepth, name);
  if (pd->depth != pd->statedepth)
    {
      pd->depth--;
      return;
    }
  pd->depth--;
  pd->statedepth--;
  switch (pd->state)
    {
    case STATE_SIZE:
    case STATE_M4SIZE:
      pd->size = (off_t)strtoull(pd->content, 0, 10);
      break;
    case STATE_HASH:
    case STATE_M4HASH:
      pd->chksum.clear();
      pd->chksum.resize(pd->chksuml, 0);
      if (strlen(pd->content) != size_t(pd->chksuml) * 2 || !hexstr2bytes(&pd->chksum[0], pd->content, pd->chksuml))
	{
	  pd->chksum.clear();
          pd->chksuml = 0;
	}
      break;
    case STATE_PHASH:
    case STATE_M4PHASH:
      if (strlen(pd->content) != size_t(pd->piecel) * 2)
	break;
      pd->piece.resize(pd->piecel * (pd->npiece + 1), 0);
      if (!hexstr2bytes(&pd->piece[pd->piecel * pd->npiece], pd->content, pd->piecel))
	{
	  pd->piece.resize(pd->piecel * pd->npiece, 0);
	  break;
	}
      pd->npiece++;
      break;
    case STATE_PIECES:
    case STATE_M4PIECES:
      if (pd->piecel == 4)
	{
	  pd->zsync = pd->piece;
	  pd->nzsync = pd->npiece;
	}
      else
	{
	  pd->sha1 = pd->piece;
	  pd->nsha1 = pd->npiece;
	}
      pd->piecel = pd->npiece = 0;
      pd->piece.clear();
      break;
    case STATE_URL:
    case STATE_M4URL:
      if (*pd->content)
	{
	  pd->urls[pd->nurls].url = string(pd->content);
	  pd->nurls++;
	}
      break;
    default:
      break;
    }
  pd->state = pd->sbtab[pd->state];
  pd->docontent = 0;
}

static void XMLCALL
characterData(void *userData, const XML_Char *s, int len)
{
  struct ml_parsedata *pd = reinterpret_cast<struct ml_parsedata *>(userData);
  int l;
  char *c;
  if (!pd->docontent)
    return;
  l = pd->lcontent + len + 1;
  if (l > pd->acontent)
    {
      pd->content = reinterpret_cast<char *>(realloc(pd->content, l + 256));
      pd->acontent = l + 256;
    }
  c = pd->content + pd->lcontent;
  pd->lcontent += len;
  while (len-- > 0)
    *c++ = *s++;
  *c = 0;
}


MetaLinkParser::MetaLinkParser()
{
  struct stateswitch *sw;
  int i;
 
  pd = new ml_parsedata();
  pd->size = off_t(-1);
  for (i = 0, sw = stateswitches; sw->from != NUMSTATES; i++, sw++)
    {
      if (!pd->swtab[sw->from])
        pd->swtab[sw->from] = sw;
      pd->sbtab[sw->to] = sw->from;
    }
  pd->content = reinterpret_cast<char *>(malloc(256));
  pd->acontent = 256;
  pd->lcontent = 0;
  pd->parser = XML_ParserCreate(NULL);
  XML_SetUserData(pd->parser, pd);
  XML_SetElementHandler(pd->parser, startElement, endElement);
  XML_SetCharacterDataHandler(pd->parser, characterData);
}

MetaLinkParser::~MetaLinkParser()
{
  XML_ParserFree(pd->parser);
  free(pd->content);
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
  if (XML_Parse(pd->parser, buf, len, 0) == XML_STATUS_ERROR)
    ZYPP_THROW(Exception("Parse Error"));
}

static bool urlcmp(const ml_url &a, const ml_url &b)
{
  return a.priority < b.priority;
}

void
MetaLinkParser::parseEnd()
{
  if (XML_Parse(pd->parser, 0, 0, 1) == XML_STATUS_ERROR)
    ZYPP_THROW(Exception("Parse Error"));
  if (pd->nurls)
    stable_sort(pd->urls.begin(), pd->urls.end(), urlcmp);
}

std::vector<Url>
MetaLinkParser::getUrls()
{
  std::vector<Url> urls;
  int i;
  for (i = 0; i < pd->nurls; ++i)
    urls.push_back(Url(pd->urls[i].url));
  return urls;
}

MediaBlockList
MetaLinkParser::getBlockList()
{
  size_t i;
  MediaBlockList bl(pd->size);
  if (pd->chksuml == 20)
    bl.setFileChecksum("SHA1", pd->chksuml, &pd->chksum[0]);
  else if (pd->chksuml == 32)
    bl.setFileChecksum("SHA256", pd->chksuml, &pd->chksum[0]);
  if (pd->size != off_t(-1) && pd->blksize)
    {
      size_t nb = (pd->size + pd->blksize - 1) / pd->blksize;
      off_t off = 0;
      size_t size = pd->blksize;
      for (i = 0; i < nb; i++)
	{
	  if (i == nb - 1)
	    {
	      size = pd->size % pd->blksize;
	      if (!size)
		size = pd->blksize;
	    }
          size_t blkno = bl.addBlock(off, size);
          if (int(i) < pd->nsha1)
	    {
	      bl.setChecksum(blkno, "SHA1", 20, &pd->sha1[20 * i]);
	      if (int(i) < pd->nzsync)
		{
		  unsigned char *p = &pd->zsync[4 * i];
		  bl.setRsum(blkno, 4, p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24, pd->blksize);
		}
	    }
	  off += pd->blksize;
	}
    }
  return bl;
}

  } // namespace media
} // namespace zypp

