/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_PS_H
#define ZYPPER_PS_H

#include <string>
class Zypper;

/*
 ps ...
*/

/** ps specific options */
struct PsOptions : public MixinOptions<ZypperCommand::PS>
{
  PsOptions()
  : _shortness( 0 )
  {}

  unsigned	_shortness;	//< 1:wo file, 2:only proc with services, 3:service names only
  std::string	_format;	//< format string for --print / shortness 3
  std::string _debugFile; //< file name for the --debugFile switch

  bool tableWithFiles() const		{ return _shortness < 1; }
  bool tableWithNonServiceProcs() const	{ return _shortness < 2; }
  bool printServiceNamesOnly() const	{ return _shortness >= 3; }
  bool debugEnabled() const {return (!_debugFile.empty());}
};

/** Execute ps.
 */
int ps( Zypper & zypper_r );

#endif // ZYPPER_PS_H
