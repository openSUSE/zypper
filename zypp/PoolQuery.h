/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ui/PoolQuery.h
 *
*/
#ifndef QUERY_H_
#define QUERY_H_

#include "zypp/ui/Selectable.h"
#include "zypp/sat/SolvAttr.h"

#include "zypp/base/Function.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////

  /**
   * Meta-data query API for user interfaces.
   * 
   * TODO: details, examples.
   */
  class PoolQuery
  {
  public:
    
    //typedef SelectableSet::iterator      ResultIterator;
    //typedef constSelectableSet::iterator constResultIterator;

    typedef function<bool( const ResObject::Ptr & )> ProcessResolvable;

    PoolQuery(PoolQuery::ProcessResolvable fnc);
    ~PoolQuery();

  public:

    /**
     * executes the query with the current settings
     * results are yielded on the callback passed on
     * construction
     */
    void execute(const std::string &term) const;

    /**
     * Filter by selectable kind.
     * 
     * By default, all kinds will be returned. If addKind() is used,
     * only the specified kinds will be returned (multiple kinds will be ORed).
     * 
     * Pass ResTraits<T>::kind to this method, where T is one of the
     * \ref Resolvable child classes (e.g. ResTraits<Pattern>::kind).
     */
    void addKind(const Resolvable::Kind &kind);

    /**
     * Filter by selectable name.
     * 
     * \param name   what to search for
     * \param isRegex is the value a regex?
     */
    //void addName(const std::string & name, bool isRegex = false);


    /**
     * Filter by the \a value of any available attribute of selectables.
     * 
     * \note Selectables of a kind not supporting the specified attribute will
     * <b>not</b> be returned.
     * 
     * \param attrid  attribute identfier (sat::SolvAttr or cache::Attribute
     *                or something implementation independent)
     * \param value   what to search for
     * \param isRegex is the value a regex?
     */
    //void addAttribute(const solv::SolvAttr & attrid,
    //                  const std::string & value,
    //                  bool isRegex = false);

    /**
     * Filter by Selectable status.
     * 
     * This should cover also plain 'is installed' and 'not installed' statuses.
     *
     * \param status Selectable status (zypp::ui::Status enum)
     */
    //void addStatus(const Status status);
    

    /**
     * Add dependency filter.
     *
     * \param dtype   depenedcy type
     * \param name    depenency name
     * \param edition edition for a versioned dependency
     * \param rel     operand for a versioned dependency
     *
     * \todo maybe a isRegexp bool as in addName() for the name parameter would
     *       be handy here as well.
     * \todo add more addDependecy() variants
     */
    //void addDependency(const Dep & dtype,
    //                   const std::string & name,
    //                   const Edition & edition = Edition(),
    //                   const Rel & rel = Rel::EQ);

    /** \name Text Attributes Matching Options */
    //@{
    /**
     * Set case sentitive on
     * ( disables \ref SEARCH_NOCASE flag )
     */
    void setCaseSensitive(const bool value = true);

    /**
     * Free function to set the satsolver repo search
       flags.

       \see SEARCH_STRINGMASK
       \see SEARCH_STRING
       \see SEARCH_SUBSTRING
       \see SEARCH_GLOB
       \see SEARCH_REGEX
       \see SEARCH_NOCASE
       \see SEARCH_NO_STORAGE_SOLVABLE
    */
    void setFlags(int flags);
    //void setLocale(const Locale & locale);
    //@}

    //void setRequireAll(const bool require_all = true);

    /** selectable iterator over the result */
    //ResultIterator resultBegin() const;
    //ResultIterator resultEnd() const; 

    /** Returns the size of the query result. */
    //size_t resultSize() const;

    /** Low-cost empty query result checker */
    //bool resultEmpty() const;
    
    // a forEach method consuming a functor can be added here, too

  public:
    class Impl;
  private:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };


/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif /*QUERY_H_*/
