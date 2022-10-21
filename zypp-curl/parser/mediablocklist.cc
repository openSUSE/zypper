/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
  \file zypp-curl/parser/mediablocklist.cc

  This file implements the delta file block reuse algorithm. If the available hashes use rolling checksum style
  hashes, we use the zsync algorithm to search for blocks that are available.

  The basic idea of zsync is that on the server side a metadata file is generated, this file contains a list of checksums
  for each block in the target file ( the file we want to download ) as well as some other important informations
  like the size of the blocks, file checksum and mirrors. The client ( we ) then can use a delta file, basically
  a older version of the file we want to download, to find reusable blocks identified by their checksum.

  A simple approach would be now to calculate all checksums in the delta file for each block, but that does not take into
  account that the blocks we want to reuse have moved in the file, e.g. a new set bytes of data was put in the beginning
  of the file, shifting all blocks to a new offset most likely not as a multiple of blocksize or worse not even a power of 2.
  Think a block is 2k but the data inserted is only 200 bytes, offsetting all blocks for 200bytes, in that case the simple approach could not reuse a single block
  of data because it can't find them.

  To work around this problem the code calculates the checksums on every possible byte offset of the delta file. To speed this process
  up when finding a block we skip forward to the end of the block. If there is a match at x, its very unlikely to find another match starting
  inside the boundaries of the current block ( between x and x + blocksize - 1 ), except if the file has a lot of redundancy.

  This process naturally can take very long if there are no, or just a few matching blocks in the two files. To speed this up
  a multi step checksum process is used. First we calculate a very cheap to calculate weak checksum for each block based on
  the Adler-32 checksum:

  \f[
    a(k,l) = (\sum_{i=k}^l X_i) \bmod M
  \f]
  \f[
    \begin{align}
    b(k,l) &= (\sum_{i=k}^l (l-i+1)X_i) \bmod M \\
           &= (\sum_{i=k}^l a(k,i) ) \bmod M
    \end{align}
  \f]
  \f[
    s(k,l) = a(k,l) + 2^{16} b(k,l)
  \f]

  where \f$s(k, l)\f$ is the rolling checksum of the bytes \f$X_k \ldots X_l\f$. For simplicity and speed, we use \f$M = 2^{16}\f$.

  The important property of this checksum is that successive values can be computed very efficiently using recurrence relations:
  \f[
    a(k+1,l+1) = (a(k,l) - X_k + X_{l+1}) \bmod M
  \f]
  \f[
    b(k+1,l+1) = (b(k,l) - (l-k+1) X_k + a(k+1,l+1)) \bmod M
  \f]

  To understand why we can use recurrence relations its important to look at the formulas written out
  for a simple example, we will ignore the modulo operation for simplicity:

  For the \a a formula is simple to see that we just need to subtract the value of the byte moving out of the block and add the
  value of the new byte coming in at the end
  \f[
    \begin{align}
    a(0,2) &= X_0 + X_1 + X_2  \\
    a(1,3) &= a(0,2) - X_0 + X_3  \\
           &= X_0 + X_1 + X_2 - X_0 + X_3  \\
           &= X_1 + X_2 + X_3
    \end{align}
  \f]

  The \a b part is a bit more complex, writing the formula out also shows us why \a b can be expressed as: \f$b=(\sum_{i=k}^l a(k,i) ) \bmod M\f$:
  \f[
    \begin{align}
    b(0,2) &= 3X_0 + 2X_1 + X_2 \\
           &= X_0 + X_0 +X_0 + X_1 + X_1 + X_2 \\
           &= X0 + X0 + X1 + X0 +X1 +X2 \\
           &= a(0,0) + a(0,1) + a(0,2) = (\sum_{i=k}^l a(k,i) )
    \end{align}
  \f]
  \f[
    \begin{align}
    b(1,3) &= b(0,2) - (2-0+1)X_0 + a(1,3) \\
           &= b(0,2) - 3X_0 + a(1,3) \\
           &= 3X_0 + 2X_1 + X_2 - 3X_0 + X_1 + X_2 + X_3 \\
           &= 3X_1 + 2X_2 + X_3
    \end{align}
  \f]

  This shows us that we can very quickly calculate the new checksum of a block at offset \a i+1 based on the checksum of the block at offset \a i.

  If the weak checksum matches, a stronger MD4 checksum is calculated for the same block to make sure we really got the block we wanted. If the strong checksum
  matches as well the block is written to the target file and removed from the list of blocks we still need to find. The read offset then jumps to the end
  of the current bock: \a offset+blocksize and continues the process from there.

  Zsync additionally can require that always a sequence of 2 neighboring blocks need to match before they are considered a match. This depends on the filesize and is specified in the zsync metadata file.
  According to the zsync docs this greatly lowers the probability of a false match and allows to send smaller checksums for the blocks, minimizing the data we need to download in the meta datafile.

  More in depth docs can be found at http://zsync.moria.org.uk/paper and https://rsync.samba.org/tech_report
*/

#include "mediablocklist.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <iostream>
#include <fstream>

#include <zypp-core/base/Logger.h>
#include <zypp-core/base/String.h>
#include <zypp-core/AutoDispose.h>

using namespace zypp::base;

namespace zypp {
  namespace media {

    namespace {
      /**
       * Zsync uses a different rsum length based on the blocksize, since we always calculate the big
       * checksum we need to cut off the bits we are not interested in
       */
      void inline truncateRsum ( unsigned int &rs, const int rsumlen )
      {
        switch(rsumlen)
        {
        case 3:
          rs &= 0xffffff;
          break;
        case 2:
          rs &= 0xffff;
          break;
        case 1:
          rs &= 0xff;
          break;
        default:
          break;
        }
      }
    }

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

std::string MediaBlockList::fileChecksumType() const
{
  return fsumtype;
}

const UByteArray &MediaBlockList::getFileChecksum()
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
  truncateRsum( rs, rsumlen );
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

UByteArray MediaBlockList::getChecksum(size_t blkno) const
{
  if ( !haveChecksum(blkno) )
    return {};

  UByteArray buf ( chksumlen, '\0' );
  memcpy( buf.data(), chksums.data()+(chksumlen * blkno), chksumlen );
  return buf;
}

std::string MediaBlockList::getChecksumType() const
{
  return chksumtype;
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

void MediaBlockList::reuseBlocks(FILE *wfp, std::string filename)
{

  zypp::AutoFILE fp;

  if ( !chksumlen ) {
    DBG << "Delta XFER: Can not reuse blocks because we have no chksumlen" << std::endl;
    return;
  }

  if ( (fp = fopen(filename.c_str(), "r")) == 0 ) {
    DBG << "Delta XFER: Can not reuse blocks, unable to open file "<< filename << std::endl;
    return;
  }
  size_t nblks = blocks.size();
  std::vector<bool> found;
  found.resize(nblks + 1);
  if (rsumlen && !rsums.empty())
    {
      size_t blksize = blocks[0].size;
      if (nblks == 1 && rsumpad && rsumpad > blksize)
        blksize = rsumpad;

      // create hash of checksums

      // calculate the size of the hashtable by setting
      // all bits to 1 up the to currently set MSB
      // if we have 00010010 we end up with 00011111
      unsigned int hm = rsums.size() * 2;
      while (hm & (hm - 1))  {
        hm &= hm - 1;
      }
      hm = hm * 2 - 1;

      // we want at least a size if 0011 1111 1111 1111
      if (hm < 16383)
        hm = 16383;

      // simple hashtable of checksums
      auto rsumHashTable = std::make_unique<unsigned int[]>( hm+1 );
      memset(rsumHashTable.get(), 0, (hm + 1) * sizeof(unsigned int));

      // insert each rsum into the hash table
      for (unsigned int i = 0; i < rsums.size(); i++)
        {
          if (blocks[i].size != blksize && (i != nblks - 1 || rsumpad != blksize))
            continue;
          unsigned int r = rsums[i];
          unsigned int h = r & hm;
          unsigned int hh = 7;
          while (rsumHashTable[h])
            h = (h + hh++) & hm;
          rsumHashTable[h] = i + 1;
        }

      // read in block by block to find matches
      // the read buffer "buf" works like a ring buffer, means that once we are done with reading a full block
      // and didn't find a match we start again at buf[0] , filling the buffer up again, rotating until we find
      // a matching block. Once we find a matching block all we need to do is check if the current offset "i"
      // is at the end of the buffer, then we can simply write the full buffer out, or if its somewhere in between
      // then the begin of our block is buf[i+1, bufsize-1] and the end buf[0,i]
      auto ringBuf = std::make_unique<unsigned char[]>( blksize );

      // we use a second buffer to read in the next block if we are required to match more than one block at the same time.
      auto buf2 = std::make_unique<unsigned char[]>( blksize );

      // when we are required to match more than one block, it is read into buf2 advancing the file pointer,
      // to make sure that we do not loose those bytes in case the match fails we remember their count and
      // start in buf2, in the next loop those will be consumed before reading from the file again
      size_t pushback = 0;
      unsigned char *pushbackp = 0;

      // use byteshift instead of multiplication if the blksize is a power of 2
      // a value is a power of 2 if  ( N & N-1 ) == 0
      int bshift = 0; // how many bytes do we need to shift
      if ((blksize & (blksize - 1)) == 0)
        for (bshift = 0; size_t(1 << bshift) != blksize; bshift++)
          ;

      // a and b are the LS and MS bytes of the checksum, calculated a rolling style Adler32 checksum
      //
      // a(k,l) = (\sum_{i=k}^l X_i) \bmod M
      unsigned short a, b;
      a = b = 0;
      memset(ringBuf.get(), 0, blksize);
      bool eof = 0;
      bool init = 1;
      int sql = nblks > 1 && chksumlen < 16 ? 2 : 1;
      while (!eof)
        {
          for (size_t i = 0; i < blksize; i++)
            {
              // get the next character from the file
              // or if there are pushback chars use those
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

              // calculate the rsum on the fly using recurrence relations, see https://rsync.samba.org/tech_report/node3.html
              // basically we subtract the checksum value of a byte the leaves the current block window and add the new incoming one
              // using this trick we do not need to calculate the full block checksum
              // the least significant part of the checksum ( lower 8 bits ) is simply the sum of all chars in the block , modulo 2^16
              // zsync uses only a 16bit type to calculate the sums and as far as i can see does not do the modulo per block as the formula
              // says it should, we might need to do the same
              int oc = ringBuf[i];
              ringBuf[i] = c;

              a += c - oc;

              // this is calculates the most significant part of the checksum, bshift should be always set since blocksize
              // should always be a power of 2
              if (bshift)
                b += a - ( oc << bshift );
              else
                // This seems to make no sense it does not even factor in the character itself
                b += 2 * blksize;

              if (init)
                {
                  // continue reading bytes until we have the full block in our buffer
                  if (size_t(i) != blksize - 1)
                    continue;
                  init = 0;
                }

              unsigned int r = ((unsigned int)a & 65535) << 16 | ((unsigned int)b & 65535);
              truncateRsum(r, rsumlen);

              unsigned int h = r & hm;
              unsigned int hh = 7;

              // go through our hashmap to find all the matching rsums
              for (; rsumHashTable[h]; h = (h + hh++) & hm)
                {
                  size_t blkno = rsumHashTable[h] - 1;

                  // does the current block match?
                  if (rsums[blkno] != r)
                    continue;
                  if (found[blkno])
                    continue;

                  // if we need to always match 2 blocks in sequence, get the next block
                  // and check its checksum
                  if (sql == 2)
                    {
                      if (eof || blkno + 1 >= nblks)
                        continue;
                      pushback = fetchnext(fp, buf2.get(), blksize, pushback, pushbackp);
                      pushbackp = buf2.get();
                      if (!pushback)
                        continue;

                      if (!checkRsum(blkno + 1, buf2.get(), blksize))
                        continue;
                    }

                  // here we have matched all blocks that we need, do the heavy checksum
                  if (!checkChecksumRotated(blkno, ringBuf.get(), blksize, i + 1))
                    continue;

                  // heavy checksum for second block
                  if (sql == 2 && !checkChecksum(blkno + 1, buf2.get(), blksize))
                    continue;

                  // write the first and second blocks if applicable
                  writeBlock(blkno, wfp, ringBuf.get(), blksize, i + 1, found);
                  if (sql == 2)
                    {
                      writeBlock(blkno + 1, wfp, buf2.get(), blksize, 0, found);
                      pushback = 0;
                      blkno++;
                    }

                  // try to continue as long as we still match blocks
                  while (!eof)
                    {
                      blkno++;
                      pushback = fetchnext(fp, buf2.get(), blksize, pushback, pushbackp);
                      pushbackp = buf2.get();
                      if (!pushback)
                        break;

                      if (!checkRsum(blkno, buf2.get(), blksize))
                        break;

                      if (!checkChecksum(blkno, buf2.get(), blksize))
                        break;

                      writeBlock(blkno, wfp, buf2.get(), blksize, 0, found);
                      pushback = 0;
                    }

                  // if we get to this part we found at least a block, skip over the current block and start reading
                  // in a full block
                  init = true;
                  memset(ringBuf.get(), 0, blksize);
                  a = b = 0;
                  i = size_t(-1);	// start with 0 on next iteration
                  break;
                }
            }
        }
    }
  else if (chksumlen >= 16)
    {
      // dummy variant, just check the checksums
      size_t bufl = 4096;
      off_t off = 0;
      auto buf = std::make_unique<unsigned char[]>( bufl );
      for (size_t blkno = 0; blkno < blocks.size(); ++blkno)
        {
          if (off > blocks[blkno].off)
            continue;
          size_t blksize = blocks[blkno].size;
          if (blksize > bufl)
            {
              bufl = blksize;
              buf = std::make_unique<unsigned char[]>( bufl );
            }
          size_t skip = blocks[blkno].off - off;
          while (skip)
            {
              size_t l = skip > bufl ? bufl : skip;
              if (fread(buf.get(), l, 1, fp) != 1)
                break;
              skip -= l;
              off += l;
            }
          if (fread(buf.get(), blksize, 1, fp) != 1)
            break;
          if (checkChecksum(blkno, buf.get(), blksize))
            writeBlock(blkno, wfp, buf.get(), blksize, 0, found);
          off += blksize;
        }
    }
  if (!found[nblks]) {
    DBG << "Delta XFER: No reusable blocks found for " << filename << std::endl;
    return;
  }
  // now throw out all of the blocks we found
  std::vector<MediaBlock> nblocks;
  std::vector<unsigned char> nchksums;
  std::vector<unsigned int> nrsums;

  size_t originalSize = 0;
  size_t newSize      = 0;
  for (size_t blkno = 0; blkno < blocks.size(); ++blkno)
    {
      const auto &blk = blocks[blkno];
      originalSize += blk.size;
      if (!found[blkno])
        {
          // still need it
          nblocks.push_back(blk);
          newSize += blk.size;
          if (chksumlen && (blkno + 1) * chksumlen <= chksums.size())
            {
              nchksums.resize(nblocks.size() * chksumlen);
              memcpy(&nchksums[(nblocks.size() - 1) * chksumlen], &chksums[blkno * chksumlen], chksumlen);
            }
          if (rsumlen && (blkno + 1) <= rsums.size())
            nrsums.push_back(rsums[blkno]);
        }
    }
  DBG << "Delta XFER: Found blocks to reuse, " << blocks.size() << " vs " << nblocks.size() << ", resused blocks: " << blocks.size() - nblocks.size() << "\n"
      << "Old transfer size: " << originalSize << " new size: " << newSize << std::endl;
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
