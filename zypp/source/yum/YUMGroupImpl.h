/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMGroupImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMGROUPIMPL_H
#define ZYPP_SOURCE_YUM_YUMGROUPIMPL_H

#include "zypp/detail/SelectionImplIf.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/Edition.h"
#include "zypp/Source.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace yum
    { //////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : YUMGroupImpl
      //
      /** Class representing a message
      */
      class YUMGroupImpl : public detail::SelectionImplIf
      {
      public:
        /** Default ctor */
        YUMGroupImpl(
	  Source & source_r,
	  const zypp::parser::yum::YUMGroupData & parsed
	);
	/** Is to be visible for user? */
	virtual bool userVisible() const;
        /** Other requested groups */
	virtual CapSet optionalReq() const;
	/** Requested packages */
	virtual CapSet defaultReq() const;
	/** */
	virtual Label summary() const;
	/** */
	virtual Text description() const;
	/** */
	virtual Text insnotify() const;
	/** */
	virtual Text delnotify() const;
	/** */
	virtual bool providesSources() const;
	/** */
	virtual Label instSrcLabel() const;
	/** */
	virtual Vendor instSrcVendor() const;
        /** */
        virtual ByteCount size() const;	
        /** */
	virtual Label category() const;
        /** */
	virtual bool visible() const;
        /** */
	virtual Label order() const;

      protected:
// _summary
// _description;
        bool _user_visible;
	CapSet _optional_req;
	CapSet _default_req;
      private:
	Source & _source;
      public:
	Source & source() const;
      };
      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMGROUPIMPL_H
