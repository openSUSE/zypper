/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_ISSUE_H_INCLUDED
#define ZYPPER_ISSUE_H_INCLUDED

#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////
/// \class Issues
/// \brief An issue (Type,Id) pair
///////////////////////////////////////////////////////////////////
struct Issue : std::pair<std::string, std::string>
{
  Issue( std::string issueType_r, std::string issueId_r );

  std::string &       type()			{ return first; }
  const std::string & type()		const	{ return first; }
  bool                anyType()		const	{ return type().empty(); }
  bool                specificType()	const	{ return !anyType(); }

  std::string &       id()			{ return second; }
  const std::string & id()		const	{ return second; }
  bool                anyId()		const 	{ return id().empty(); }
  bool                specificId()	const	{ return !anyId(); }
};


#endif
