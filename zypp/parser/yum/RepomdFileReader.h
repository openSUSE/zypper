/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/RepomdFileReader.h
 * Interface of repomd.xml file reader.
 */
#ifndef zypp_source_yum_RepomdFileReader_H
#define zypp_source_yum_RepomdFileReader_H

#include <set>

#include <zypp/base/PtrTypes.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/Function.h>

#include <zypp/OnMediaLocation.h>

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Reads through a repomd.xml file and collects type, location, checksum and
   * other data about metadata files to be processed.
   *
   * After each file entry is read, an \ref OnMediaLocation and the resource type
   * string are prepared and passed to the \ref _callback.
   */
  class RepomdFileReader : private base::NonCopyable
  {
  public:
    /** Callback taking \ref OnMediaLocation and the resource type string */
    typedef function< bool( OnMediaLocation &&, const std::string & )> ProcessResource;

   /**
    * CTOR. Creates also \ref xml::Reader and starts reading.
    *
    * \param repomd_file is the repomd.xml file you want to read
    * \param callback is a function.
    *
    * \see RepomdFileReader::ProcessResource
    */
    RepomdFileReader( const Pathname & repomd_file, const ProcessResource & callback );
    /** \overload Quick parsing keywords and keyhints. */
    RepomdFileReader( const Pathname & repomd_file );

    /** DTOR */
    ~RepomdFileReader();

  public:
    /** repo keywords parsed on the fly */
    const std::set<std::string> & keywords() const;

    /** gpg key hits shipped in keywords (bsc#1184326) */
    std::vector<std::pair<std::string,std::string>> keyhints() const;

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // namespace yum
  } // namespace parser
} // namespace zypp

#endif // zypp_source_yum_RepomdFileReader_H
