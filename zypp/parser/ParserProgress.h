/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_ParserProgress_H
#define ZYPP_ParserProgress_H

#include "boost/shared_ptr.hpp"
#include "boost/function.hpp"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace parser
{ /////////////////////////////////////////////////////////////////

  class ParserProgress
  {
    public:
      typedef boost::shared_ptr<ParserProgress> Ptr;
      ParserProgress( boost::function<void (int)> fnc )
      : _fnc(fnc)
      {
        
      };
      ~ParserProgress()
      {};
      void progress(int p)
      {
        if (_fnc)
          _fnc(p);
      }
      
    private:
      boost::function<void (int)> _fnc;
      int _file_size;
      int no_lines;
  };

} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ParserProgress_H
