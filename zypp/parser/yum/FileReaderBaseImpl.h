/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/FileReaderBaseImpl.h
 * Implementation interface of shared code for yum::*FileReaders.
 */
#ifndef ZYPP_PARSER_YUM_FILEREADERBASEIMPL_H_
#define ZYPP_PARSER_YUM_FILEREADERBASEIMPL_H_

#include "zypp/data/ResolvableData.h"
#include "zypp/parser/xml/Reader.h"

#include "zypp/parser/yum/FileReaderBase.h"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  class FileReaderBase::BaseImpl : private base::NonCopyable
  {
  public:

    /**
     * Enumeration of some YUM metadata xml tags.
     */
    enum Tag
    {
      tag_NONE,
      tag_package,
      tag_format,
      tag_patch,
      tag_atoms,
      tag_script,
      tag_message,
      tag_pkgfiles,
      tag_deltarpm,
      tag_patchrpm,
      tag_pattern,
      tag_product
    };

    /**
     * Lightweight object for remembering currently processed tag.
     * 
     * Usage:
     * 
     * - call \ref append() on opening tag
     * - use \ref contains() to check if the given tag is being processed
     * - call \ref remove() on closing tag
     * 
     * \note the above works only if the path elements are unique. To support
     *   also non-unique elements, an equals() method will need to be implemented
     *   and used instead of \ref contains().
     * 
     * \note tags you want to use with TagPath must be enumerated in \ref Tag
     * enum.
     */
    struct TagPath
    {
      typedef std::list<Tag> TagList;
      
      void append(const Tag tag) { path.push_back(tag); }
      void remove() { if (!path.empty()) path.pop_back(); }
      unsigned depth() const { return path.size(); }
      Tag currentTag() const
      {
        if (!path.empty()) return path.back();
        return tag_NONE;
      }
      bool contains(const Tag tag) const
      {
        TagList::const_iterator result = find(path.begin(), path.end(), tag);
        return result != path.end();
      }
      std::string asString();

      TagList path;
    };


  public:
    /** CTOR */
    BaseImpl();

    /**
     * Process package node and its subtree.
     * This method can be extended for specific implementations.
     *
     * \param reader_r
     * \param package_ptr This can be either data::Package or data::SrcPackage.
     * \return true if the package node or current subnode has been consumed
     *         (no further processing is required), false otherwise.
     * \throws ParseException if a serious error is encountered while parsing
     */
    bool consumePackageNode(xml::Reader & reader_r, data::Packagebase_Ptr & package_ptr);

    /**
     * Function for processing all <code>format</code> tag subtree nodes.
     * 
     * \return true if the package node or current subnode has been consumed
     *         (no further processing is required), false otherwise.
     */
    bool consumeFormatNode(xml::Reader & reader_r, data::Packagebase_Ptr & package_ptr);

    /**
     * Processes RPM dependency tags (rpm:entry, rpm:requires, ...).
     * 
     * \return true if a dependency tag has been encountered, false otherwise.
     * \throws ParseException if a serious error is encountered while parsing
     */
    bool consumeDependency(xml::Reader & reader_r, data::Dependencies & deps_r);


  public:
    /** Appends \a tag to \ref _tagpath. */
    void tag(const Tag tag) { _tagpath.append(tag); }

    /** Check whether we are currently processing given \a tag. */
    bool isBeingProcessed(Tag tag) const { return _tagpath.contains(tag); }

    /** Move to parent tag in the \ref _tagpath. */
    void toParentTag() { _tagpath.remove(); }

    /** Getter of the tagpath object. */
    const TagPath & tagPath() const { return _tagpath; }


  private:
    /** Used to remember the tag beeing currently processed. */ 
    TagPath _tagpath;

    /**
     * Used to remember whether we are expecting an rpm:entry tag
     * e.g. for rpm:requires
     */
    bool _expect_rpm_entry;

    /**
     * Type of dependecy beeing processed.
     */
    Dep _dtype;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*ZYPP_PARSER_YUM_FILEREADERBASEIMPL_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
