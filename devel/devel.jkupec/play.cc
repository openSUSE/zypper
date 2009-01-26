#include <iostream>

#include "zypp/base/LogTools.h"

#include "zypp/parser/HistoryLogReader.h"
#include "zypp/parser/ParseException.h"

using std::endl;
using std::cout;
using namespace zypp;

struct HistoryItemCollector
{
//  vector<HistoryItem::Ptr> items;

  bool operator()( const HistoryItem::Ptr & item_ptr )
  {
//    items.insert(item);
    cout << *item_ptr << endl;
    return true;
  }
};

// ---------------------------------------------------------------------------

int main( int argc, const char * argv[] )
{
  --argc; ++argv; // skip arg 0

  HistoryItemCollector ic;
  parser::HistoryLogReader reader(*argv, ic);

  try
  {
    reader.readAll();
  }
  catch (const parser::ParseException & e)
  {
    cout << "error in " << *argv << ":" << endl;
    cout << e.asUserHistory() << endl;
  }

  cout << "done" << endl;
  return 0;
}
