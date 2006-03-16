/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMAtomImpl.h
 *
*/
#ifndef ZYPP_TARGET_XMLSTORE_ATOMIMPL_H
#define ZYPP_TARGET_XMLSTORE_ATOMIMPL_H

#include "zypp/source/SourceImpl.h"
#include "zypp/detail/AtomImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
    namespace storage
    { //////////////////////////////////////////////////////////////

      
      /** Class representing a Atom
      */
      class XMLAtomImpl : public detail::AtomImplIf
      {
      public:
        /** Default ctor */
        XMLAtomImpl();
      private:
	
      public:
	//Source_Ref source() const;
      };
      ///////////////////////////////////////////////////////////////////
    } // namespace storage
    /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMATOMIMPL_H
