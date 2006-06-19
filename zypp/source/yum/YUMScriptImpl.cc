/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMScriptImpl.cc
*/

#include "zypp/source/yum/YUMScriptImpl.h"
#include "zypp/Arch.h"
#include "zypp/Edition.h"
#include "zypp/base/Gettext.h"

#include "zypp/source/yum/YUMSourceImpl.h"

#include <fstream>


using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace yum
    {

      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : YUMScriptImpl
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
      */
      YUMScriptImpl::YUMScriptImpl(
	Source_Ref source_r,
	const zypp::parser::yum::YUMPatchScript & parsed
      )
      : _do_script(parsed.do_script)
      , _undo_script(parsed.undo_script)
      , _do_location(parsed.do_location)
      , _undo_location(parsed.undo_location)
      , _do_media(1)
      , _undo_media(1)
      , _do_checksum(parsed.do_checksum_type, parsed.do_checksum)
      , _undo_checksum(parsed.undo_checksum_type, parsed.undo_checksum)
      , _source(source_r)
      {
	unsigned do_media = strtol(parsed.do_media.c_str(), 0, 10);
	if (do_media > 0)
	  _do_media = do_media;
	unsigned undo_media = strtol(parsed.undo_media.c_str(), 0, 10);
	if (undo_media > 0)
	  _undo_media = undo_media;
      }

      Pathname YUMScriptImpl::do_script() const {
	if (_do_script != "")
	{
	  _tmp_file = filesystem::TmpFile();
	  Pathname pth = _tmp_file.path();
	  ofstream st(pth.asString().c_str());
	  st << _do_script << endl;
	  return pth;
	}
	else if (_do_location != "" && _do_location != "/")
	{
	  Pathname script = source().provideFile(_do_location, _do_media);
          if (! filesystem::is_checksum(script, _do_checksum))
	  {
	    ZYPP_THROW(Exception(N_("Failed check for the script file check sum")));
	  }
	  return script;
	}
	else
	{
	  return Pathname();
	}
      }
      /** Get the script to undo the change */
     Pathname YUMScriptImpl::undo_script() const {
	if (_undo_script != "")
	{
	  _tmp_file = filesystem::TmpFile();
	  Pathname pth = _tmp_file.path();
	  ofstream st(pth.asString().c_str());
	  st << _undo_script << endl;
	  return pth;
	}
	else if (_undo_location != "" && _undo_location != "/")
	{
	  Pathname script = source().provideFile(_undo_location, _undo_media);
          if (! filesystem::is_checksum(script, _undo_checksum) )
	  {
	    ZYPP_THROW(Exception(N_("Failed check for the script file check sum")));
	  }
	  return script;
	}
	else return Pathname();
      }
      /** Check whether script to undo the change is available */
      bool YUMScriptImpl::undo_available() const {
	return _undo_script != ""
	  || (_undo_location != "" && _undo_location != "/");
      }

      Source_Ref YUMScriptImpl::source() const
      { return _source; }



    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
