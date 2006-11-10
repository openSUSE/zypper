/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/PatternExpander.h
 *
*/
#ifndef ZYPP_UI_PATTERNEXPANDER_H
#define ZYPP_UI_PATTERNEXPANDER_H

#include <iosfwd>
#include <map>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/base/Iterator.h"

#include "zypp/Pattern.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class ResPool;

  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatternExpander
    //
    /** Recursively expand a Pattern.
     *
     * This means recursively expanding Patterns included by this or
     * extending this. The result is a \c set of <tt>Pattern::constPtr</tt>
     * accessible via iterator.
    */
    class PatternExpander : private base::NonCopyable
    {
    private:
      class Impl;
      typedef std::map<Pattern::constPtr,
                       DefaultIntegral<bool, false> > PatternMap;

    public:
      typedef PatternMap::size_type size_type;
      typedef PatternMap::key_type  value_type;
      typedef MapKVIteratorTraits<PatternMap>::Key_const_iterator const_iterator;

    public:
      /** Ctor taking the ResPool to use. */
      PatternExpander( const ResPool & pool_r );

    public:
      /** \name Expand a Pattern.
       * \returns Number of Patterns after expansion.
      */
      //@{
      size_type expand( const ResObject::constPtr & obj_r )
      { return expand( asKind<Pattern>(obj_r) ); }

      size_type expand( const Pattern::constPtr & pat_r );
      //@}

      /** \name Access result of last expansion.. */
      //@{
      size_type size() const;

      bool empty() const;

      const_iterator begin() const;

      const_iterator end() const;
      //@}

    private:
      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_PATTERNEXPANDER_H
