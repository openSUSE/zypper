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
  virtual std::string category() const;
  virtual Label vendor() const;
  virtual TranslatedText summary() const;
  virtual TranslatedText description() const;
  virtual std::list<std::string> flags() const;
  virtual TranslatedText shortName() const;
  virtual std::string distributionName() const;
  virtual Edition distributionEdition() const;
protected:
  std::string _category;
  Label _vendor;
  TranslatedText _summary;
  TranslatedText _description;
  TranslatedText _short_name;
  std::string _distribution_name;
  Edition _distribution_edition;
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
