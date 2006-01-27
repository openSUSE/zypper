/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMMessageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMMESSAGEIMPL_H
#define ZYPP_SOURCE_YUM_YUMMESSAGEIMPL_H

#include "zypp/detail/MessageImpl.h"
#include "zypp/parser/yum/YUMParserData.h"

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
      //        CLASS NAME : YUMMessageImpl
      //
      /** Class representing a message
      */
      class YUMMessageImpl : public detail::MessageImplIf
      {
      public:
        /** Default ctor */
        YUMMessageImpl(
	  Source & source_r,
	  const zypp::parser::yum::YUMPatchMessage & parsed
	);
	/** Get the text of the message */
	virtual std::string text() const;
	/** Get the type of the message (YesNo / OK) */
	virtual std::string type() const;
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


      protected:
	/** The text of the message */
	std::string _text;
	/** The type of the message (YesNo / OK) */
	std::string _type;
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
#endif // ZYPP_SOURCE_YUM_YUMMESSAGEIMPL_H
