#ifndef ZYPPNG_IO_IOBUFFER_P_H
#define ZYPPNG_IO_IOBUFFER_P_H

#include <zypp/zyppng/core/ByteArray>
#include <vector>
#include <cstdint>

namespace zyppng {

  class IOBuffer {

    struct Chunk {
      ByteArray _buffer;

      unsigned head = 0;
      unsigned tail = 0;

      char * data () {
        return _buffer.data() + head;
      }

      const char * data () const {
        return _buffer.data() + head;
      }

      unsigned available() const {
        return _buffer.size() - tail;
      }
      unsigned len () const {
        return tail - head;
      }
    };

  public:
    IOBuffer( unsigned chunkSize = 0);
    char *reserve( size_t bytes );
    char *front ();
    size_t frontSize () const;
    void clear     ( );
    size_t discard ( size_t bytes );
    void chop ( size_t bytes );
    void append ( const char *data, size_t count );
    void append ( const ByteArray &data );
    size_t read ( char *buffer, size_t max );
    size_t size ( ) const;
    std::vector<Chunk>::size_type chunks ()  const;
    inline int64_t indexOf ( const char c ) const { return indexOf( c, size() ); }
    int64_t indexOf ( const char c, size_t maxCount, size_t pos = 0 ) const;
    ByteArray readLine ( const size_t max = 0 );
    bool canReadLine () const;

  private:
    unsigned _defaultChunkSize;
    std::vector<Chunk> _chunks;
  };

}

#endif
