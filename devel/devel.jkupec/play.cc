#include <iostream>

#include "zypp/base/LogTools.h"
#include "zypp/ZYppCallbacks.h"

#include "zypp/parser/HistoryLogReader.h"
#include "zypp/parser/ParseException.h"

using std::endl;
using std::cout;
using namespace zypp;

bool progress_function(const ProgressData & p)
{
  cout << ".";
  return true;
}

struct HistoryItemCollector
{
  std::vector<HistoryItem::Ptr> items;

  bool operator()( const HistoryItem::Ptr & item_ptr )
  {
    items.push_back(item_ptr);
    //cout << *item_ptr << endl;
    return true;
  }
};

// ---------------------------------------------------------------------------

int main( int argc, const char * argv[] )
{
  --argc; ++argv; // skip arg 0

  HistoryItemCollector ic;
  parser::HistoryLogReader reader(*argv, boost::ref(ic));
  reader.setIgnoreInvalidItems(true);
  ProgressReportReceiver progress;
  progress.connect();
  try
  {
    //reader.readAll(&progress_function);
    reader.readFromTo(Date("2009-01-01", "%Y-%m-%d"), Date("2009-01-02", "%Y-%m-%d"));
  }
  catch (const parser::ParseException & e)
  {
    cout << "error in " << *argv << ":" << endl;
    cout << e.asUserHistory() << endl;
  }
  progress.disconnect();

  cout << "got " << ic.items.size() << endl;
  return 0;
}
