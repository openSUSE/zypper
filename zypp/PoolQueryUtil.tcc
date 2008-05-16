/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/PoolQueryUtil.tcc
 *
 * including fstream is not hell here because this header only included
 * by implementation file, header doesn't need include it.
*/
#ifndef ZYPP_POOLQUERYUTIL_TCC
#define ZYPP_POOLQUERYUTIL_TCC

#include <fstream>

#include "zypp/Pathname.h"
#include "zypp/PoolQuery.h"
#include "zypp/base/String.h"

namespace zypp
{

  /**
   * sends to output iterator all queries readed from file.
   *
   * \code
   *  list<PoolQuery> s;
   *  insert_iterator<list<PoolQuery> > ii(s, s.end());
   *  readPoolQueriesFromStream(f,ii);
   * \endcode
   */
  template <class OutputIterator>
  void readPoolQueriesFromFile(const zypp::filesystem::Pathname &file,
      OutputIterator out )
  {
    bool found;
    std::ifstream fin( file.c_str() );

    if (!fin)
      ZYPP_THROW(Exception(str::form("Cannot open file %s",file.c_str())));

    do
    {
      zypp::PoolQuery q;
      found = q.recover( fin );
      if (found)
        *out++ = q;
    } while ( found );

    fin.close();
  }

  /**
   * Writes all queries from begin to end.
   */

  template <class InputIterator>
  void writePoolQueriesToFile(const zypp::filesystem::Pathname &file,
      InputIterator begin, InputIterator end )
  {
    std::ofstream fout( file.c_str(), std::ios_base::out | std::ios_base::trunc );

    if (!fout)
      ZYPP_THROW(Exception(str::form("Cannot open file %s",file.c_str())));

    for_( it, begin, end )
    {
      it->serialize( fout );
    }

    fout.close();
  }

}
#endif // ZYPP_POOLQUERYUTIL_H
