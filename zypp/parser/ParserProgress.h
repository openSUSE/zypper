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
       * the total_steps to the goal, and report using the same
       * unit, then
       */
      ParserProgress( boost::function<void (long int)> fnc, long int total_steps = 100 )
      : _fnc(fnc), _previous_progress(0), _total_steps(total_steps)
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
      void progress(long int p)
      {
        //std::cout << "real " << p << std::endl;
        if ( _total_steps != 100 )
        {
          long int current_done = p;
          p = (long int)(((double) current_done/(double) _total_steps)*100);
        }
        
        if (_fnc && ( p !=  _previous_progress ))
        {
          _previous_progress = p;
          _fnc(p);
        }
      }
      
      void setTotalSteps( long int total_steps )
      {
        _total_steps = total_steps;
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
      boost::function<void (long int)> _fnc;
      long int _previous_progress;
      long int _total_steps;
  };

} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ParserProgress_H
