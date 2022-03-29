#ifndef ZYPPNG_IO_IOBUFFER_P_H
#define ZYPPNG_IO_IOBUFFER_P_H

#include <zypp-core/zyppng/core/ByteArray>
#include <vector>
#include <cstdint>

namespace zyppng {

  class IOBuffer {

    struct Chunk {
      ByteArray _buffer;

      int64_t head = 0;
      int64_t tail = 0;

      char * data () {
        return _buffer.data() + head;
      }

      const char * data () const {
        return _buffer.data() + head;
      }

      int64_t available() const {
        return _buffer.size() - tail;
      }
      int64_t len () const {
        return tail - head;
      }
    };

  public:
    IOBuffer( int64_t chunkSize = 0 );

    char *reserve( int64_t bytes );
    char *front ();
    int64_t frontSize () const;
    void clear     ( );
    int64_t discard( int64_t bytes );
    void chop ( int64_t bytes );
    void append ( const char *data, int64_t count );
    void append ( const ByteArray &data );
    int64_t read ( char *buffer, int64_t max );
    int64_t size ( ) const;
    std::vector<Chunk>::size_type chunks ()  const;
    inline int64_t indexOf ( const char c ) const { return indexOf( c, size() ); }
    int64_t indexOf (const char c, int64_t maxCount, int64_t pos = 0 ) const;
    ByteArray readLine ( const int64_t max = 0 );
    int64_t readLine( char *buffer, int64_t max );
    bool canReadLine () const;

  private:
    int64_t _defaultChunkSize;
    std::vector<Chunk> _chunks;
  };

}

#endif
