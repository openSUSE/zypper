/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/DiskUsage.h
 *
*/
#ifndef ZYPP_DISKUSAGE_H
#define ZYPP_DISKUSAGE_H

#include <set>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class DiskUsage {
  public:
    /**
    * @short Holds data about how much space will be needed per directory.
    **/
    struct Entry {
      Entry() : _size(0), _files(0) {};
      Entry(const std::string& path_r,
              const unsigned size_r = 0,
              const unsigned files_r = 0)
      : path(path_r), _size(size_r), _files(files_r)
      {
        // assert leading and trailing '/'
        if ( ! path.empty() )
        {
          if ( *path.begin() != '/' ) path.insert( path.begin(), '/' );
          if ( *path.rbegin() != '/' ) path.insert( path.end(), '/' );
        }
      }
      std::string path;
      mutable unsigned _size;
      mutable unsigned _files;
      friend std::ostream & operator<<( std::ostream & str, const Entry & obj );
      /**
       * Test for equality based on directory name.
       **/
      bool operator==( const Entry & rhs ) const {
        return  path == rhs.path;
      }
      /**
       * Order based on directory name.
       **/
      bool operator<( const Entry & rhs ) const {
        return  path < rhs.path;
      }
      /**
       * Return true if this entry denotes a directory equal to or below rhs._dirname.
       **/
      bool isBelow( const Entry & rhs ) const {
        // whether _dirname has prefix rhs._dirname
        return( path.compare( 0, rhs.path.size(), rhs.path ) == 0 );
      }
      /**
       * Return true if this entry denotes a directory equal to or below dirname_r.
       **/
      bool isBelow( const std::string & dirname_r ) const {
        return  isBelow( Entry( dirname_r ) );
      }

      /**
       * Numerical operation based on size and files values.
       **/
      const Entry & operator+=( const Entry & rhs ) const {
        _size  += rhs._size;
        _files += rhs._files;
        return *this;
      }
      /**
       * Numerical operation based on size and files values.
       **/
      const Entry & operator-=( const Entry & rhs ) const {
        _size  -= rhs._size;
        _files -= rhs._files;
        return *this;
      }
    };
  private:
    typedef std::set<Entry> EntrySet;
    EntrySet _dirs;
  public:

    DiskUsage() {};
   /**
     * Add an entry. If already present, sum up the new entries size and files value.
     **/
    void add( const Entry & newent_r ) {
      std::pair<EntrySet::iterator,bool> res = _dirs.insert( newent_r );
      if ( !res.second ) {
        *res.first += newent_r;
      }
    }
   /**
     * Add an entry. If already present, sum up the new entries size and files value.
     **/
    void add( const std::string & dirname_r, const unsigned & size_r = 0, const unsigned & files_r = 0 ) {
      add( Entry( dirname_r, size_r, files_r ) );
    }
    /**
     * Whether there is no entry available.
     */
    bool empty() const { return _dirs.empty(); }
    /**
     * Number of entries
     **/
    unsigned size() const { return _dirs.size(); }
    /**
     * Clear EntrySet
     **/
    void clear() { _dirs.clear(); }
    /**
     * Sum up any entries for dirname_r and its descendants and remove them
     * on the fly. Return the result.
     **/
    Entry extract( const std::string & dirname_r );

  public:

    typedef EntrySet::iterator               iterator;
    typedef EntrySet::reverse_iterator       reverse_iterator;

    /**
     * Forward iterator pointing to the first entry (if any)
     **/
    iterator begin() { return _dirs.begin(); }
    /**
     * Forward iterator pointing behind the last entry.
     **/
    iterator end() { return _dirs.end(); }
    /**
     * Reverse iterator pointing to the last entry (if any)
     **/
    reverse_iterator rbegin() { return _dirs.rbegin(); }
    /**
     * Reverse iterator pointing before the first entry.
     **/
    reverse_iterator rend() { return _dirs.rend(); }

    typedef EntrySet::const_iterator         const_iterator;
    typedef EntrySet::const_reverse_iterator const_reverse_iterator;

    /**
     * Forward const iterator pointing to the first entry (if any)
     **/
    const_iterator begin() const { return _dirs.begin(); }
    /**
     * Forward const iterator pointing behind the last entry.
     **/
    const_iterator end() const { return _dirs.end(); }
    /**
     * Reverse const iterator pointing to the last entry (if any)
     **/
    const_reverse_iterator rbegin() const { return _dirs.rbegin(); }
    /**
     * Reverse const iterator pointing before the first entry.
     **/
    const_reverse_iterator rend()const { return _dirs.rend(); }

  public:

    friend std::ostream & operator<<( std::ostream & str, const DiskUsage & obj );

  };
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DISKUSAGE_H
