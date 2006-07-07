/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#ifndef ZYPP_Signature_H
#define ZYPP_Signature_H

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  
  class Signature
  {
    public:
    Signature();
    ~Signature();
    
    /** Overload to realize stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;
    
    private:
  };  
  
  /** \relates Signature Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Signature & obj )
  { return obj.dumpOn( str ); }  
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_Signature_H
