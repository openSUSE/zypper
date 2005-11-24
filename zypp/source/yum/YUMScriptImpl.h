/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMScriptImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMSCRIPTIMPL_H
#define ZYPP_SOURCE_YUM_YUMSCRIPTIMPL_H

#include "zypp/detail/ScriptImpl.h"
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
      //        CLASS NAME : YUMScriptImpl
      //
      /** Class representing an update script */
      class YUMScriptImpl : public detail::ScriptImplIf
      {
      public:
        /** Default ctor */
        YUMScriptImpl( const zypp::parser::yum::YUMPatchScript & parsed );
	/** Get the script to perform the change */
	virtual std::string do_script() const;
	/** Get the script to undo the change */
	virtual std::string undo_script() const;
	/** Check whether script to undo the change is available */
	virtual bool undo_available() const;

	virtual Label summary() const;
	virtual Text description() const;
	virtual Text insnotify() const;
	virtual Text delnotify() const;
	virtual bool providesSources() const;
	virtual Label instSrcLabel() const;
	virtual Vendor instSrcVendor() const;


      protected:
	/** The script to perform the change */
	std::string _do_script;
	/** The script to undo the change */
	std::string _undo_script;
      };
      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMSCRIPTIMPL_H
