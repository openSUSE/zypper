/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/ProvideRes.h
 */
#ifndef ZYPP_MEDIA_PROVIDERES_H_INCLUDED
#define ZYPP_MEDIA_PROVIDERES_H_INCLUDED

#include <zypp-media/ng/ProvideFwd>
#include <zypp-core/Pathname.h>
#include <zypp-core/ManagedFile.h>
#include <memory>


namespace zyppng
{

  struct ProvideResourceData;

  /**
   * \class ProvideRes
   * A ProvideRes object is a reference counted ownership of a resource in the cache provided by
   * a \ref Provide instance.
   * It is generally advisable to release a ProvideRes instance asap, to make sure resources
   * can be released and devices are free to be ejected.
   *
   * \note all ProvideRes instances will become invalid when the \ref Provide instance is
   *       released.
   */
  class ProvideRes
  {
  public:
    ProvideRes( std::shared_ptr<ProvideResourceData> dataPtr );
    virtual ~ProvideRes();

    /*!
     * Returns the path to the provided file
     */
    const zypp::Pathname file () const;

    /*!
     * Returns a reference to the currently held media handle, this can be a invalid handle
     */
    const ProvideMediaHandle &mediaHandle () const;

    private:
      std::shared_ptr<ProvideResourceData> _data;
  };
}


#endif
