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

#include "zypp/source/SourceImpl.h"
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
    Source_Ref source_r,
    const zypp::parser::yum::YUMPatternData & parsed
  );
  /** Is to be visible for user? */
  virtual bool userVisible() const;
  /** */
  virtual TranslatedText summary() const;
  /** */
  virtual TranslatedText description() const;
  /** */
  virtual bool isDefault() const;
  /** */
  virtual TranslatedText category() const;
  /** */
  virtual Pathname icon() const;
  /** */
  virtual Pathname script() const;


protected:
  bool _user_visible;
  TranslatedText _summary;
  TranslatedText _description;
  bool _default;
  TranslatedText _category;
  Pathname _icon;
  Pathname _script;
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
#endif // ZYPP_SOURCE_YUM_YUMPATTERNIMPL_H
