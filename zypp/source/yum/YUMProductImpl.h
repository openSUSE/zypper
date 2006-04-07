/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMProductImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMPRODUCTIMPL_H
#define ZYPP_SOURCE_YUM_YUMPRODUCTIMPL_H

#include "zypp/source/SourceImpl.h"
#include "zypp/detail/ProductImpl.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/source/yum/YUMSourceImpl.h"

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
      //        CLASS NAME : YUMProductImpl
      //
      /** Class representing a patch
      */
      class YUMProductImpl : public detail::ProductImplIf
      {
      public:
        /** Default ctor */
        YUMProductImpl(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMProductData & parsed
	);
	std::string category() const;
	Label vendor() const;
	TranslatedText summary() const;
	TranslatedText description() const;
	Text insnotify() const;
	Text delnotify() const;
	ByteCount size() const;
	bool providesSources() const;
	Label instSrcLabel() const;
	Vendor instSrcVendor() const;
	std::list<std::string> flags() const;
	TranslatedText shortName() const;
      protected:
	std::string _category;
	Label _vendor;
	TranslatedText _summary;
	TranslatedText _description;
	TranslatedText _short_name;


      private:
	Source_Ref _source;
      public:
	Source_Ref source() const;

       };
      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMPRODUCTIMPL_H
