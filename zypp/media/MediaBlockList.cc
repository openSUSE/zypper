/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaBlockList.cc
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <iostream>
#include <fstream>

#include <zypp/media/MediaBlockList.h>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>

using namespace zypp::base;

namespace zypp {
  namespace media {

MediaBlockList::MediaBlockList(off_t size)
{
  filesize = size;
  haveblocks = false;
  chksumlen = 0;
  chksumpad = 0;
  rsumlen = 0;
  rsumpad = 0;
}

size_t
MediaBlockList::addBlock(off_t off, size_t size)
{
  haveblocks = true;
  blocks.push_back(MediaBlock( off, size ));
  return blocks.size() - 1;
}

void
MediaBlockList::setFileChecksum(std::string ctype, int cl, unsigned char *c)
{
  if (!cl)
    return;
  fsumtype = ctype;
  fsum.resize(cl);
  memcpy(&fsum[0], c, cl);
}

const std::vector<unsigned char> &MediaBlockList::getFileChecksum()
{
  return fsum;
}

bool
MediaBlockList::createFileDigest(Digest &digest) const
{
  return digest.create(fsumtype);
}

bool
MediaBlockList::verifyFileDigest(Digest &digest) const
{
  if (!haveFileChecksum())
    return true;
  std::vector<unsigned char>dig = digest.digestVector();
  if (dig.empty() || dig.size() < fsum.size())
    return false;
  return memcmp(&dig[0], &fsum[0], fsum.size()) ? false : true;
}

void
MediaBlockList::setChecksum(size_t blkno, std::string cstype, int csl, unsigned char *cs, size_t cspad)
{
  if (!csl)
    return;
  if (!chksumlen)
    {
      if (blkno)
	return;
      chksumlen = csl;
      chksumtype = cstype;
      chksumpad = cspad;
    }
  if (csl != chksumlen || cstype != chksumtype || cspad != chksumpad || blkno != chksums.size() / chksumlen)
    return;
  chksums.resize(chksums.size() + csl);
  memcpy(&chksums[csl * blkno], cs, csl);
}

void
MediaBlockList::setRsum(size_t blkno, int rsl, unsigned int rs, size_t rspad)
{
  if (!rsl)
    return;
  if (!rsumlen)
    {
      if (blkno)
	return;
      rsumlen = rsl;
      rsumpad = rspad;
    }
  if (rsl != rsumlen || rspad != rsumpad || blkno != rsums.size())
    return;
  rsums.push_back(rs);
}

bool
MediaBlockList::createDigest(Digest &digest) const
{
  return digest.create(chksumtype);
}

bool
MediaBlockList::verifyDigest(size_t blkno, Digest &digest) const
{
  if (!haveChecksum(blkno))
    return true;
  size_t size = blocks[blkno].size;
  if (!size)
    return true;
  if (chksumpad > size)
    {
      char pad[chksumpad - size];
      memset(pad, 0, chksumpad - size);
      digest.update(pad, chksumpad - size);
    }
  std::vector<unsigned char>dig = digest.digestVector();
  if (dig.empty() || dig.size() < size_t(chksumlen))
    return false;
  return memcmp(&dig[0], &chksums[chksumlen * blkno], chksumlen) ? false : true;
}

unsigned int
MediaBlockList::updateRsum(unsigned int rs, const char* bytes, size_t len) const
{
  if (!len)
    return rs;
  unsigned short s, m;
  s = (rs >> 16) & 65535;
  m = rs & 65535;
  for (; len > 0 ; len--)
    {
      unsigned short c = (unsigned char)*bytes++;
      s += c;
      m += s;
    }
  return (s & 65535) << 16 | (m & 65535);
}

bool
MediaBlockList::verifyRsum(size_t blkno, unsigned int rs) const
{
  if (!haveRsum(blkno))
    return true;
  size_t size = blocks[blkno].size;
  if (!size)
    return true;
  if (rsumpad > size)
    {
      unsigned short s, m;
      s = (rs >> 16) & 65535;
      m = rs & 65535;
      m += s * (rsumpad - size);
      rs = (s & 65535) << 16 | (m & 65535);
    }
  switch(rsumlen)
    {
    case 3:
      rs &= 0xffffff;
    case 2:
      rs &= 0xffff;
    case 1:
      rs &= 0xff;
    default:
      break;
    }
  return rs == rsums[blkno];
}

bool
MediaBlockList::checkRsum(size_t blkno, const unsigned char *buf, size_t bufl) const
{
  if (blkno >= blocks.size() || bufl < blocks[blkno].size)
    return false;
  unsigned int rs = updateRsum(0, (const char *)buf, blocks[blkno].size);
  return verifyRsum(blkno, rs);
}

bool
MediaBlockList::checkChecksum(size_t blkno, const unsigned char *buf, size_t bufl) const
{
  if (blkno >= blocks.size() || bufl < blocks[blkno].size)
    return false;
  Digest dig;
  if (!createDigest(dig))
    return false;
  dig.update((const char *)buf, blocks[blkno].size);
  return verifyDigest(blkno, dig);
}

std::vector<unsigned char> MediaBlockList::getChecksum(size_t blkno) const
{
  if ( !haveChecksum(blkno) )
    return {};

  std::vector<unsigned char> buf ( chksumlen, '\0' );
  memcpy( buf.data(), chksums.data()+(chksumlen * blkno), chksumlen );
  return buf;
}

// specialized version of checkChecksum that can deal with a "rotated" buffer
bool
MediaBlockList::checkChecksumRotated(size_t blkno, const unsigned char *buf, size_t bufl, size_t start) const
{
  if (blkno >= blocks.size() || bufl < blocks[blkno].size)
    return false;
  if (start == bufl)
    start = 0;
  Digest dig;
  if (!createDigest(dig))
    return false;
  size_t size = blocks[blkno].size;
  size_t len = bufl - start > size ? size : bufl - start;
  dig.update((const char *)buf + start, len);
  if (size > len)
    dig.update((const char *)buf, size - len);
  return verifyDigest(blkno, dig);
}

// write block to the file. can also deal with "rotated" buffers
void
MediaBlockList::writeBlock(size_t blkno, FILE *fp, const unsigned char *buf, size_t bufl, size_t start, std::vector<bool> &found) const
{
  if (blkno >= blocks.size() || bufl < blocks[blkno].size)
    return;
  off_t off = blocks[blkno].off;
  size_t size = blocks[blkno].size;
  if (fseeko(fp, off, SEEK_SET))
    return;
  if (start == bufl)
    start = 0;
  size_t len = bufl - start > size ? size : bufl - start;
  if (fwrite(buf + start, len, 1, fp) != 1)
    return;
  if (size > len && fwrite(buf, size - len, 1, fp) != 1)
    return;
  found[blkno] = true;
  found[blocks.size()] = true;
}

static size_t
fetchnext(FILE *fp, unsigned char *bp, size_t blksize, size_t pushback, unsigned char *pushbackp)
{
  size_t l = blksize;
  int c;

  if (pushback)
    {
      if (pushbackp != bp)
        memmove(bp, pushbackp, pushback);
      bp += pushback;
      l -= pushback;
    }
  while (l)
    {
      c = getc(fp);
      if (c == EOF)
        break;
      *bp++ = c;
      l--;
    }
  if (l)
    memset(bp, 0, l);
  return blksize - l;
}


void
MediaBlockList::reuseBlocks(FILE *wfp, std::string filename)
{
  FILE *fp;

  if (!chksumlen || (fp = fopen(filename.c_str(), "r")) == 0)
    return;
  size_t nblks = blocks.size();
  std::vector<bool> found;
  found.resize(nblks + 1);
  if (rsumlen && !rsums.empty())
    {
      size_t blksize = blocks[0].size;
      if (nblks == 1 && rsumpad && rsumpad > blksize)
	blksize = rsumpad;
      // create hash of checksums
      unsigned int hm = rsums.size() * 2;
      while (hm & (hm - 1))
	hm &= hm - 1;
      hm = hm * 2 - 1;
      if (hm < 16383)
	hm = 16383;
      unsigned int *ht = new unsigned int[hm + 1];
      memset(ht, 0, (hm + 1) * sizeof(unsigned int));
      for (unsigned int i = 0; i < rsums.size(); i++)
	{
	  if (blocks[i].size != blksize && (i != nblks - 1 || rsumpad != blksize))
	    continue;
	  unsigned int r = rsums[i];
	  unsigned int h = r & hm;
	  unsigned int hh = 7;
	  while (ht[h])
	    h = (h + hh++) & hm;
	  ht[h] = i + 1;
	}

      unsigned char *buf = new unsigned char[blksize];
      unsigned char *buf2 = new unsigned char[blksize];
      size_t pushback = 0;
      unsigned char *pushbackp = 0;
      int bshift = 0;
      if ((blksize & (blksize - 1)) == 0)
	for (bshift = 0; size_t(1 << bshift) != blksize; bshift++)
	  ;
      unsigned short a, b;
      a = b = 0;
      memset(buf, 0, blksize);
      bool eof = 0;
      bool init = 1;
      int sql = nblks > 1 && chksumlen < 16 ? 2 : 1;
      while (!eof)
	{
	  for (size_t i = 0; i < blksize; i++)
	    {
	      int c;
	      if (eof)
		c = 0;
	      else
		{
		   if (pushback)
		    {
		      c = *pushbackp++;
		      pushback--;
		    }
		  else
		    c = getc(fp);
		  if (c == EOF)
		    {
		      eof = true;
		      c = 0;
		      if (!i || sql == 2)
			break;
		    }
		}
	      int oc = buf[i];
	      buf[i] = c;
	      a += c - oc;
	      if (bshift)
		b += a - (oc << bshift);
	      else
		b += a - oc * blksize;
	      if (init)
		{
		  if (size_t(i) != blksize - 1)
		    continue;
		  init = 0;
		}
	      unsigned int r;
	      if (rsumlen == 1)
		r = ((unsigned int)b & 255);
	      else if (rsumlen == 2)
		r = ((unsigned int)b & 65535);
	      else if (rsumlen == 3)
		r = ((unsigned int)a & 255) << 16 | ((unsigned int)b & 65535);
	      else
		r = ((unsigned int)a & 65535) << 16 | ((unsigned int)b & 65535);
	      unsigned int h = r & hm;
	      unsigned int hh = 7;
	      for (; ht[h]; h = (h + hh++) & hm)
		{
		  size_t blkno = ht[h] - 1;
		  if (rsums[blkno] != r)
		    continue;
		  if (found[blkno])
		    continue;
		  if (sql == 2)
		    {
		      if (eof || blkno + 1 >= nblks)
			continue;
		      pushback = fetchnext(fp, buf2, blksize, pushback, pushbackp);
		      pushbackp = buf2;
		      if (!pushback)
			continue;
		      if (!checkRsum(blkno + 1, buf2, blksize))
			continue;
		    }
		  if (!checkChecksumRotated(blkno, buf, blksize, i + 1))
		    continue;
		  if (sql == 2 && !checkChecksum(blkno + 1, buf2, blksize))
		    continue;
		  writeBlock(blkno, wfp, buf, blksize, i + 1, found);
		  if (sql == 2)
		    {
		      writeBlock(blkno + 1, wfp, buf2, blksize, 0, found);
		      pushback = 0;
		      blkno++;
		    }
		  while (!eof)
		    {
		      blkno++;
		      pushback = fetchnext(fp, buf2, blksize, pushback, pushbackp);
		      pushbackp = buf2;
		      if (!pushback)
			break;
		      if (!checkRsum(blkno, buf2, blksize))
			break;
		      if (!checkChecksum(blkno, buf2, blksize))
			break;
		      writeBlock(blkno, wfp, buf2, blksize, 0, found);
		      pushback = 0;
		    }
		  init = false;
		  memset(buf, 0, blksize);
	 	  a = b = 0;
		  i = size_t(-1);	// start with 0 on next iteration
		  break;
		}
	    }
	}
      delete[] buf2;
      delete[] buf;
      delete[] ht;
    }
  else if (chksumlen >= 16)
    {
      // dummy variant, just check the checksums
      size_t bufl = 4096;
      off_t off = 0;
      unsigned char *buf = new unsigned char[bufl];
      for (size_t blkno = 0; blkno < blocks.size(); ++blkno)
	{
	  if (off > blocks[blkno].off)
	    continue;
	  size_t blksize = blocks[blkno].size;
	  if (blksize > bufl)
	    {
	      delete[] buf;
	      bufl = blksize;
              buf = new unsigned char[bufl];
	    }
	  size_t skip = blocks[blkno].off - off;
	  while (skip)
	    {
	      size_t l = skip > bufl ? bufl : skip;
	      if (fread(buf, l, 1, fp) != 1)
		break;
	      skip -= l;
	      off += l;
	    }
	  if (fread(buf, blksize, 1, fp) != 1)
	    break;
	  if (checkChecksum(blkno, buf, blksize))
	    writeBlock(blkno, wfp, buf, blksize, 0, found);
	  off += blksize;
	}
    }
  if (!found[nblks])
    return;
  // now throw out all of the blocks we found
  std::vector<MediaBlock> nblocks;
  std::vector<unsigned char> nchksums;
  std::vector<unsigned int> nrsums;

  for (size_t blkno = 0; blkno < blocks.size(); ++blkno)
    {
      if (!found[blkno])
	{
	  // still need it
	  nblocks.push_back(blocks[blkno]);
	  if (chksumlen && (blkno + 1) * chksumlen <= chksums.size())
	    {
	      nchksums.resize(nblocks.size() * chksumlen);
	      memcpy(&nchksums[(nblocks.size() - 1) * chksumlen], &chksums[blkno * chksumlen], chksumlen);
	    }
	  if (rsumlen && (blkno + 1) <= rsums.size())
	    nrsums.push_back(rsums[blkno]);
	}
    }
  blocks = nblocks;
  chksums = nchksums;
  rsums = nrsums;
}

std::string
MediaBlockList::asString() const
{
  std::string s;
  size_t i, j;

  if (filesize != off_t(-1))
    {
      long long size = filesize;
      s = zypp::str::form("[ BlockList, file size %lld\n", size);
    }
  else
    s = "[ BlockList, filesize unknown\n";
  if (!haveblocks)
    s += "  No block information\n";
  if (chksumpad)
    s += zypp::str::form("  Checksum pad %zd\n", chksumpad);
  if (rsumpad)
    s += zypp::str::form("  Rsum pad %zd\n", rsumpad);
  for (i = 0; i < blocks.size(); ++i)
    {
      long long off=blocks[i].off;
      long long size=blocks[i].size;
      s += zypp::str::form("  (%8lld, %8lld)", off, size);
      if (chksumlen && chksums.size() >= (i + 1) * chksumlen)
	{
	  s += "  " + chksumtype + ":";
	  for (j = 0; j < size_t(chksumlen); j++)
	    s += zypp::str::form("%02hhx", chksums[i * chksumlen + j]);
	}
      if (rsumlen && rsums.size() > i)
	{
	  s += "  RSUM:";
	  s += zypp::str::form("%0*x", 2 * rsumlen, rsums[i]);
	}
      s += "\n";
    }
  s += "]";
  return s;
}

  } // namespace media
} // namespace zypp

