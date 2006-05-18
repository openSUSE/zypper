/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/DefaultFalseBool.h
 *
*/
#ifndef ZYPP_BASE_DefaultFalseBool_H
#define ZYPP_BASE_DefaultFalseBool_H

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : DefaultFalseBool<_Tp>
    //
    /** Bool whose default value is false
     *
     * Useful for map<T, bool> where if the entry is not present
     * you get a undefined bool
     *
    */
      class DefaultFalseBool
      {
        public:
        /** DefaultCtor */
        DefaultFalseBool() : _b(false)
        {}
        
        /** Dtor */
        ~DefaultFalseBool()
        {}
         
        operator bool() const
        { return _b; }
        
        bool operator =(bool a)
        {
          _b = a;
          return a;
        }
        
        private:
        bool _b;
      };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_DefaultFalseBool_H
