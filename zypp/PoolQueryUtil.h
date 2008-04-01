// including fstream is not hell here because this header only included
// by implementation file, header doesn't need include it
// also that's why this file is not protected by IFNDEF (you get it
// only when you explicitly need it in implementation)
#include <fstream>

#include "zypp/Pathname.h"
#include "zypp/PoolQuery.h"
  
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
      return; //TODO exception

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
      return; //TODO exception

    for_( it, begin, end )
    {
      it->serialize( fout );
    }

    fout.close();
  }
