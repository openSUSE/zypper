/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMPatternImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMPATTERNIMPL_H
#define ZYPP_SOURCE_YUM_YUMPATTERNIMPL_H

#include "zypp/detail/PatternImplIf.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/Edition.h"

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
      //        CLASS NAME : YUMPatternImpl
      //
      /** Class representing a message
      */
      class YUMPatternImpl : public detail::PatternImplIf
      {
      public:
        /** Default ctor */
        YUMPatternImpl(
	  Source & source_r,
	  const zypp::parser::yum::YUMPatternData & parsed
	);
	/** Is to be visible for user? */
	virtual bool userVisible() const;
        /** optional requirements */
	virtual CapSet optionalReq() const;
	/** default requirements */
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
#endif // ZYPP_SOURCE_YUM_YUMPATTERNIMPL_H
