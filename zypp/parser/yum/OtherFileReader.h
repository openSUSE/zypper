/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/OtherFileReader.h
 * Interface of other.xml.gz file reader.
 */
#ifndef OTHERFILEREADER_H_
#define OTHERFILEREADER_H_

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/base/Function.h"

#include "zypp/ProgressData.h"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Reads through a other.xml file and collects additional package data
   * (changelogs).
   *
   * After a package is read, a \ref data::Resolvable
   * and \ref Changelog is prepared and \ref _callback
   * is called with these two objects passed in.
   *
   * The \ref _callback is provided on construction.
   *
   * \code
   * OtherFileReader reader(other_file,
   *                        bind(&SomeClass::callbackfunc, &SomeClassInstance, _1, _2));
   * \endcode
   */
  class OtherFileReader : private base::NonCopyable
  {
  public:
    /**
     * Callback definition.
     */
    typedef function<bool(const data::Resolvable_Ptr &, const Changelog)> ProcessPackage;

    /**
     * CTOR. Creates also \ref xml::Reader and starts reading.
     * 
     * \param other_file the other.xml.gz file you want to read
     * \param callback function to process \ref _resolvable data.
     * \param progress progress reporting object
     *
     * \see OtherFileReader::ProcessPackage
     */
    OtherFileReader(
      const Pathname & other_file,
      const ProcessPackage & callback,
      const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc());
    
    /**
     * DTOR
     */
    ~OtherFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // ns zypp
  } // ns parser
} // ns yum

#endif /*OTHERFILEREADER_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
