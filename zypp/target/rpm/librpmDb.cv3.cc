/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/librpmDb.cv3.cc
 *
 */
#include <iostream>

#include "zypp/base/Logger.h"

#include "zypp/target/rpm/librpmDb.h"
#include "zypp/target/rpm/RpmCallbacks.h"
#include "zypp/ZYppCallbacks.h"

using namespace std;

#undef Y2LOG
#define Y2LOG "librpmDb"

namespace zypp
{
namespace target
{
namespace rpm
{
/******************************************************************
*
*
*	FUNCTION NAME : convertV3toV4
*
* \throws RpmException
*
*/
void convertV3toV4( const Pathname & v3db_r, const librpmDb::constPtr & v4db_r )
{
  // report
  callback::SendReport<ConvertDBReport> report;
  report->start(v3db_r);
  try
  {
    // Does no longer work with rpm 4.9.
    // internal_convertV3toV4( v3db_r, v4db_r, report );
    INT << "Unsupported: Convert rpm3 database to rpm4" << endl;
    ZYPP_THROW(RpmDbOpenException(Pathname("/"), v3db_r));
  }
  catch (RpmException & excpt_r)
  {
    report->finish(v3db_r, ConvertDBReport::FAILED,excpt_r.asUserString());
    ZYPP_RETHROW(excpt_r);
  }
  report->finish(v3db_r, ConvertDBReport::NO_ERROR, "");
}

} // namespace rpm
} // namespace target
} // namespace zypp
