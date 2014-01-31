/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Application.h
 */
#ifndef ZYPP_APPLICATION_H
#define ZYPP_APPLICATION_H

#include <iosfwd>

#include "zypp/ResObject.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  DEFINE_PTR_TYPE(Application);

  ///////////////////////////////////////////////////////////////////
  /// \class Application
  /// \brief Class representing an application (appdata.xml)
  ///////////////////////////////////////////////////////////////////
  class Application : public ResObject
  {
  public:
    typedef Application              Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    // no attributes defined by now

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Application( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Application();
  };

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_APPLICATION_H
