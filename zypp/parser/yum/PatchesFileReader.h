/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/PatchesFileReader.h
 * Interface of patches.xml file reader.
 */
#ifndef zypp_source_yum_PatchesFileReader_H
#define zypp_source_yum_PatchesFileReader_H

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Function.h"


namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Iterates through a patches.xml file giving on each iteration
   * a \ref OnMediaLocation object with the resource and its
   * patch id.
   * The iteration is done via a callback provided on
   * construction.
   *
   * \code
   * PatchesFileReader reader(patches_file, 
   *                  bind( &SomeClass::callbackfunc, &object, _1, _2 ) );
   * \endcode
   */
  class PatchesFileReader : private base::NonCopyable
  {
  public:

    /**
     * Callback definition
     * first parameter is a \ref OnMediaLocation object with the resource
     * second parameter is the patch id.
     */
    typedef
      function<bool( const OnMediaLocation &, const std::string & )>
      ProcessResource;


    /**
    * CTOR. Creates also \ref xml::Reader and starts reading.
    * 
     * \param patches_file is the patches.xml file you want to read
     * \param callback is a function.
     * 
     * \see PatchesFileReader::ProcessResource
     */
    PatchesFileReader(const Pathname &patches_file,
                      const ProcessResource & callback);

    /**
     * DTOR
     */
    ~PatchesFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*zypp_source_yum_PatchesFileReader_H*/

// vim: set ts=2 sts=2 sw=2 et ai:
