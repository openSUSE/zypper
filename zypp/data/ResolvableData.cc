/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/data/ResolvableData.h"

using namespace std;

namespace zypp
{
namespace data
{
 
IMPL_PTR_TYPE(Resolvable);  
IMPL_PTR_TYPE(ResObject);
IMPL_PTR_TYPE(Script);
IMPL_PTR_TYPE(Message);
IMPL_PTR_TYPE(Selection);  
IMPL_PTR_TYPE(Pattern);
  

std::ostream& operator<<(std::ostream& out, const ResObject &data)
{
      out << "[ " << data.name << " " << data.edition << " ]" << endl;
      return out;
//       << "  provides: " << data.provides << endl
//       << "  conflicts: " << data.conflicts << endl
//       << "  obsoletes: " << data.obsoletes << endl
//       << "  freshens: " << data.freshens << endl
//       << "  requires: " << data.requires << endl
//       << "  recommends:" << endl << data.recommends << endl
//       << "  suggests:" << endl << data.suggests << endl
//       << "  supplements:" << endl << data.supplements << endl
//       << "  enhances:" << endl << data.enhances << endl
}

/*
std::ostream& operator<<(std::ostream& out, const zypp::shared_ptr<AtomBase> data)
{
  out << "Atom data" << endl;
  switch (data->atomType())
  {
    case AtomBase::TypePackage:
      out << "  atom type: " << "package" << endl
          << *zypp::dynamic_pointer_cast<Patch>(data);
      break;
    case AtomBase::TypeMessage:
      out << "  atom type: " << "message" << endl
          << *zypp::dynamic_pointer_cast<Message>(data);
      break;
    case AtomBase::TypeScript:
      out << "  atom type: " << "script" << endl
          << *zypp::dynamic_pointer_cast<Script>(data);
      break;
    default:
      out << "Unknown atom type" << endl;
  }
  return out;
}  
  
std::ostream& operator<<(std::ostream& out, const Script& data)
{
      out << "  do script: " << data.do_script << endl
      << "  undo script: " << data.undo_script << endl
      << "  do script location: " << data.do_location << endl
      << "  undo script location: " << data.undo_location << endl
      << "  do script media: " << data.do_media << endl
      << "  undo script media: " << data.undo_media << endl
      << "  do checksum type: " << data.do_checksum_type << endl
      << "  do checksum: " << data.do_checksum << endl
      << "  undo checksum type: " << data.undo_checksum_type << endl
      << "  undo checksum: " << data.undo_checksum << endl;
  return out;
}  
  

std::ostream& operator<<(std::ostream& out, const Message& data)
{
  out << "Message Data: " << endl
      << "  name: " << data.name << endl
      << "  epoch: " << data.epoch << endl
      << "  version: " << data.ver << endl
      << "  release: " << data.rel << endl
      << "  provides: " << data.provides << endl
      << "  conflicts: " << data.conflicts << endl
      << "  obsoletes: " << data.obsoletes << endl
      << "  freshens: " << data.freshens << endl
      << "  requires: " << data.requires << endl
      << "  recommends:" << endl << data.recommends << endl
      << "  suggests:" << endl << data.suggests << endl
      << "  supplements:" << endl << data.supplements << endl
      << "  enhances:" << endl << data.enhances << endl
      << "  text: " << data.text << endl;
  return out;
}
*/
  
} // namespace cache
} // namespace zypp
