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
      
      /**
       * initializes a progress objetc, with a callback functor
       * if you are not reporting percentage, then set
       * the metric to the goal, and report using the same
       * unit, then
       */
      ParserProgress( boost::function<void (int)> fnc, int metric = 100 )
      : _fnc(fnc), _previous_progress(0), _metric(metric)
      {
        
      };
      
      ~ParserProgress()
      {};
      
      /**
       * report progress, which in most cases
       * executes the functor associated with
       * this progress object to update progress
       * information
       */
      void progress(int p)
      {
        if ( _metric != 100 )
        {
          int current_done = p;
          p = (int)((float) current_done/(float) _metric);
        }
        
        if (_fnc && ( p !=  _previous_progress ))
        {
          _previous_progress = p;
          _fnc(p);
        }
      }
      
      /**
       * report progress finished
       */
      void finish()
      {
        int p = 100;
        if (_fnc && ( p !=  _previous_progress ))
        {
          _previous_progress = p;
          _fnc(p);
        }
      }
      
      /**
       * report progress started
       */
      void start()
      {
        int p = 0;
        if (_fnc && ( p !=  _previous_progress ))
        {
          _previous_progress = p;
          _fnc(p);
        }
      }
      
    private:
      boost::function<void (int)> _fnc;
      int _previous_progress;
      int _metric;
  };

} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ParserProgress_H
