/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_BASE_Random_H
#define ZYPP_BASE_Random_H

#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { ///////////////////////////
    // Taken from KApplication
    int random_int();
    // Taken from KApplication
    std::string random_string(int length);


    /** Return a random number from <tt>[0,RAND_MAX[</tt>. */
    inline unsigned random()
    {
      return random_int();
    }
    /** Return a random number from <tt>[0,size_r[</tt>. */
    inline unsigned random( unsigned size_r )
    {
      return random_int() % size_r;
    }
    /** Return a random number from <tt>[min_r,min_r+size_r[</tt>. */
    inline unsigned random( unsigned min_r, unsigned size_r )
    {
      return min_r + random( size_r );
    }


  } //ns base
} // ns zypp

#endif

