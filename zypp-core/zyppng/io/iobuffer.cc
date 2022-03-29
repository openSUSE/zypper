#include "private/iobuffer_p.h"
#include <cstring>
#include <cassert>

namespace zyppng {

  enum {
    DefChunkSize = 4096
  };

  IOBuffer::IOBuffer( int64_t chunkSize ) : _defaultChunkSize ( chunkSize == 0 ? DefChunkSize : chunkSize )
  { }

  char *IOBuffer::reserve( int64_t bytes )
  {
    assert( bytes > 0 && size_t(bytes) < ByteArray::maxSize() );
    // do we need a new chunk?
    if ( _chunks.size() ) {
      auto &back = _chunks.back();
      if ( back.available() >= bytes ) {
        char * ptr = back._buffer.data() + back.tail;
        back.tail += bytes;
        return ptr;
      }
    }

    // not enough space ready allocate a new one
    _chunks.push_back( Chunk{} );
    auto &back = _chunks.back();
    back._buffer.insert( back._buffer.end(), std::max<int64_t>( _defaultChunkSize, bytes ), '\0' );
    back.tail += bytes;
    return back.data();
  }

  char *IOBuffer::front()
  {
    if ( frontSize() == 0 )
      return nullptr;

    return _chunks.front().data();
  }

  int64_t IOBuffer::frontSize() const
  {
    if ( _chunks.empty() )
      return 0;
    return _chunks.front().len();
  }

  void IOBuffer::clear()
  {
    _chunks.clear();
  }

  int64_t IOBuffer::discard( int64_t bytes )
  {
    const int64_t bytesToDiscard = std::min(bytes, size());
    if ( bytesToDiscard == size() ) {
      clear();
      return bytesToDiscard;
    }

    int64_t discardedSoFar = 0;

    // since the chunks might not be used completely we need to iterate over them
    // counting how much used bytes we actually discard until we hit the requested amount
    while ( discardedSoFar < bytesToDiscard ) {
      auto &chunk = _chunks.front();
      const auto bytesInChunk = chunk.len();

      if ( discardedSoFar + bytesInChunk > bytesToDiscard ) {
        chunk.head += ( bytesToDiscard - discardedSoFar );
        discardedSoFar = bytesToDiscard;
      } else {
        _chunks.erase( _chunks.begin() );
        discardedSoFar += bytesInChunk;
      }
    }


    return bytesToDiscard;
  }

  /*!
   * Removes bytes from the end of the buffer
   */
  void IOBuffer::chop( int64_t bytes )
  {
    if ( bytes == 0 )
      return;

    bytes = std::min( bytes, size() );
    if ( bytes == size() ) {
      clear();
      return;
    }

    int64_t choppedSoFar = 0;
    while ( choppedSoFar < bytes && _chunks.size() ) {
      auto bytesStillToChop =  bytes - choppedSoFar;
      auto &chunk = _chunks.back();

      if ( chunk.len() > bytesStillToChop ) {
        chunk.tail -= bytesStillToChop;
        break;
      } else {
        choppedSoFar += chunk.len();
        _chunks.pop_back();
      }
    }
  }

  void IOBuffer::append(const char *data, int64_t count)
  {
    assert( count > 0 && size_t(count) < ByteArray::maxSize() );

    char *buf = reserve( count );
    if ( count == 1 )
      *buf = *data;
    else {
      ::memcpy( buf, data, count );
    }
  }

  void IOBuffer::append(const ByteArray &data)
  {
    append( data.data(), data.size() );
  }

  int64_t IOBuffer::read( char *buffer, int64_t max )
  {
    const size_t bytesToRead = std::min( size(), max );
    size_t readSoFar = 0;

    while ( readSoFar < bytesToRead && _chunks.size() ) {

      auto &chunk = _chunks.front();
      const auto toRead   = std::min<size_t>( bytesToRead - readSoFar, chunk.len() );
      ::memcpy( buffer+readSoFar, chunk.data(), toRead );
      readSoFar += toRead;

      // if we consumed all data in the chunk discard it
      chunk.head += toRead;
      if( chunk.head >= chunk.tail )
        _chunks.erase( _chunks.begin() );
    }

    return readSoFar;
  }

  int64_t IOBuffer::size() const
  {
    int64_t s = 0;
    for ( const auto &c : _chunks )
      s+= c.len();
    return s;
  }

  std::vector<IOBuffer::Chunk>::size_type IOBuffer::chunks() const
  {
    return _chunks.size();
  }

  int64_t IOBuffer::indexOf(const char c, int64_t maxCount, int64_t pos ) const
  {
    if ( maxCount == 0 )
      return -1;

    maxCount = std::min<size_t>( maxCount, size() );

    int64_t  scannedSoFar  = 0;
    for ( const auto &chunk : _chunks ) {

      //as long as pos is still after the current chunk just increase the count
      if ( scannedSoFar+chunk.len() - 1 < pos ) {
        scannedSoFar += chunk.len();
        continue;
      }

      const char * const chunkBegin = chunk.data();
      const char *s = chunkBegin;

      size_t lengthToScan = std::min<size_t>( chunk.len() , maxCount - scannedSoFar );
      if ( pos > 0  && scannedSoFar < pos ) {
        const auto adjust = (pos-scannedSoFar);
        s += adjust;
        lengthToScan -= adjust;
      }

      const char *ptr = reinterpret_cast<const char*>(::memchr( s, c, lengthToScan ));
      if ( ptr ) {
        return ( ( ptr - chunkBegin ) + scannedSoFar );
      }

      scannedSoFar += chunk.len();
      if ( scannedSoFar >= maxCount )
        break;
    }
    return -1;
  }

  ByteArray IOBuffer::readLine( const int64_t max )
  {
    assert( ( max >= 2 || max == 0 ) && size_t(max) <= ByteArray::maxSize() );

    const auto idx = indexOf( '\n', max == 0 ? size() : max );
    if ( idx == -1 )
      return {};

    zyppng::ByteArray b( idx+1, '\0' );
    read( b.data(), idx+1 );
    return b;
  }

  int64_t IOBuffer::readLine( char *buffer, int64_t max )
  {
    assert( buffer != nullptr && max > 1 );
    const auto maxRead = max - 1;
    const auto idx = indexOf( '\n', maxRead );
    const auto bytesRead = read( buffer, idx == -1 ? maxRead : idx + 1  );
    buffer[bytesRead] = '\0';
    return bytesRead;
  }

  bool IOBuffer::canReadLine() const
  {
    return indexOf('\n') >= 0;
  }

}
