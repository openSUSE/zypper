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
     * Returns a reference to the internally used managed file instance.
     * \note If you obtain this for a file that is inside the providers working directory ( e.g. a provide result for a download ),
     *       the continued use after the Provide instance was relased is undefined behaviour and not supported!
     */
    const zypp::ManagedFile & asManagedFile () const;

    /*!
     * Returns a reference to the currently held media handle, this can be a invalid handle
     */
    const ProvideMediaHandle &mediaHandle () const;

    /*!
     * The URL this ressource was provided from, can be empty
     */
    const zypp::Url &resourceUrl () const;

    /*!
     * All headers that were received from the worker when sending the result
     */
    const HeaderValueMap &headers () const;

    private:
      std::shared_ptr<ProvideResourceData> _data;
  };
}


#endif
