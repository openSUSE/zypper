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

#include "zypp/source/SourceImpl.h"
#include "zypp/detail/ScriptImpl.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/TmpPath.h"
#include "zypp/CheckSum.h"

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
        YUMScriptImpl(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchScript & parsed
	);
	/** Get the script to perform the change */
	virtual Pathname do_script() const;
	/** Get the script to undo the change */
	virtual Pathname undo_script() const;
	/** Check whether script to undo the change is available */
	virtual bool undo_available() const;

      protected:
	/** The script to perform the change */
	std::string _do_script;
	/** The script to undo the change */
	std::string _undo_script;
	/** Location of external do script on the medium */
	Pathname _do_location;
	/** Location of external undo script on the medium */
	Pathname _undo_location;
	/** Media number of the do script */
	unsigned _do_media;
	/** Media number of the undo script */
	unsigned _undo_media;

        mutable shared_ptr<filesystem::TmpFile> _tmp_do_script;
        mutable shared_ptr<filesystem::TmpFile> _tmp_undo_script;

	CheckSum _do_checksum;
	CheckSum _undo_checksum;
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
#endif // ZYPP_SOURCE_YUM_YUMSCRIPTIMPL_H
