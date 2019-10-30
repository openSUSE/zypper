/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include <iostream>
#include "issue.h"

Issue::Issue(std::string issueType_r, std::string issueId_r)
  : std::pair<std::string, std::string>( std::move(issueType_r), std::move(issueId_r) )
{}

std::ostream & operator<<( std::ostream & str, const Issue & obj )
{ return str << "Issue(" << (obj.anyType()?"ANY":obj.type()) << "#" << (obj.anyId()?"ANY":obj.id()) << ")"; }
