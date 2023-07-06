/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* Strictly for internal use!
*/

#ifndef ZYPP_TUI_OUTPUT_ALIVE_CURSOR_H
#define ZYPP_TUI_OUTPUT_ALIVE_CURSOR_H

#include <iostream>

namespace ztui {

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

}

#endif

