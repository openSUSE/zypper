
#ifndef ALIVE_CURSOR_H
#define ALIVE_CURSOR_H

#include <iostream>

class AliveCursor
{
  public:
  friend std::ostream & operator<<( std::ostream & str, const AliveCursor & obj );
  
  AliveCursor() : _current('-')
  {}
  
  char current() const
  { return _current; }
  
  void increase()
  {
    switch ( _current )
    {
      case '-':
        _current = '\\';
      break;
      case '\\':
        _current = '|';
      break;
      case '|':
        _current = '/';
      break;
      case '/':
        _current = '-';
      break;
    }
  }
  
  AliveCursor & operator++()
  {
    increase();
    return *this;
  }
  
  AliveCursor & operator++(int)
  {
    increase();
    return *this;
  }
  
  AliveCursor & done()
  {
    _current = '*';
    return *this;
  }
  
  private:
  char _current;
};

/** \relates Date Stream output */
inline std::ostream & operator<<( std::ostream & str, const AliveCursor & obj )
{ return str << obj.current(); }

#endif

