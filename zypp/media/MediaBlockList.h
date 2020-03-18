/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaBlockList.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIABLOCKLIST_H
#define ZYPP_MEDIA_MEDIABLOCKLIST_H

#include <sys/types.h>
#include <vector>

#include <zypp/Digest.h>

namespace zypp {
  namespace media {

/**
 * a single block from the blocklist, consisting of an offset and a size
 **/
struct MediaBlock {
  MediaBlock( off_t off_r, size_t size_r )
  : off( off_r )
  , size( size_r )
  {}
  off_t off;
  size_t size;
};

class MediaBlockList {
public:
  MediaBlockList(off_t filesize=off_t(-1));

  /**
   * do we have a blocklist describing the file?
   * set to true when addBlock() is called
   **/
  inline bool haveBlocks() const {
    return haveblocks;
  }
  /**
   * add a block with offset off and size size to the block list. Note
   * that blocks must be ordered and must not overlap. returns the
   * block number.
   **/
  size_t addBlock(off_t off, size_t size);

  /**
   * return the offset/size of a block with number blkno
   **/
  inline MediaBlock getBlock(size_t blkno) const {
    return blocks[blkno];
  }
  /**
   * return the number of blocks in the blocklist
   **/
  inline size_t numBlocks() const {
    return blocks.size();
  }

  /**
   * set / return the size of the whole file
   **/
  inline void setFilesize(off_t newfilesize=off_t(-1)) {
    filesize = newfilesize;
  }
  inline off_t getFilesize() const {
    return filesize;
  }
  inline bool haveFilesize() const {
    return filesize != off_t(-1);
  }

  /**
   * set / verify the checksum over the whole file
   **/
  void setFileChecksum(std::string ctype, int cl, unsigned char *c);
  const std::vector<unsigned char> &getFileChecksum( );
  bool createFileDigest(Digest &digest) const;
  bool verifyFileDigest(Digest &digest) const;
  inline bool haveFileChecksum() const {
    return !fsumtype.empty() && fsum.size();
  }

  /**
   * set / verify the (strong) checksum over a single block
   **/
  void setChecksum(size_t blkno, std::string cstype, int csl, unsigned char *cs, size_t cspad=0);
  bool checkChecksum(size_t blkno, const unsigned char *buf, size_t bufl) const;
  std::vector<unsigned char> getChecksum( size_t blkno ) const;
  bool createDigest(Digest &digest) const;
  bool verifyDigest(size_t blkno, Digest &digest) const;
  inline bool haveChecksum(size_t blkno) const {
    return chksumlen && chksums.size() >= chksumlen * (blkno + 1);
  }

  /**
   * set / verify the (weak) rolling checksum over a single block
   **/
  void setRsum(size_t blkno, int rsl, unsigned int rs, size_t rspad=0);
  bool checkRsum(size_t blkno, const unsigned char *buf, size_t bufl) const;
  unsigned int updateRsum(unsigned int rs, const char *bytes, size_t len) const;
  bool verifyRsum(size_t blkno, unsigned int rs) const;
  inline bool haveRsum(size_t blkno) const {
    return rsumlen && rsums.size() >= blkno + 1;
  }

  /**
   * scan a file for blocks from our blocklist. if we find a suitable block,
   * it is removed from the list
   **/
  void reuseBlocks(FILE *wfp, std::string filename);

  /**
   * return block list as string
   **/
  std::string asString() const;

private:
  void writeBlock(size_t blkno, FILE *fp, const unsigned char *buf, size_t bufl, size_t start, std::vector<bool> &found) const;
  bool checkChecksumRotated(size_t blkno, const unsigned char *buf, size_t bufl, size_t start) const;

  off_t filesize;
  std::string fsumtype;
  std::vector<unsigned char> fsum;

  bool haveblocks;
  std::vector<MediaBlock> blocks;

  std::string chksumtype;
  int chksumlen;
  size_t chksumpad;
  std::vector<unsigned char> chksums;

  std::string rsumtype;
  int rsumlen;
  size_t rsumpad;
  std::vector<unsigned int> rsums;
};

inline std::ostream & operator<<(std::ostream &str, const MediaBlockList &bl)
{ return str << bl.asString(); }

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIABLOCKLIST_H

