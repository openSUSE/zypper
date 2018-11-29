/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "issue.h"

Issue::Issue(std::string issueType_r, std::string issueId_r)
  : std::pair<std::string, std::string>( std::move(issueType_r), std::move(issueId_r) )
{}
