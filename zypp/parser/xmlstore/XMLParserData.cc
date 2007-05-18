/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xmlstore/XMLParserData.cc
 *
*/

#include <iostream>
#include <zypp/parser/xmlstore/XMLParserData.h>

using namespace std;

namespace zypp {
  namespace parser {
    namespace xmlstore {

      XMLDependency::XMLDependency(const std::string& kind,
                                   const std::string& encoded)
      : kind(kind),
      encoded(encoded)
      { };


      XMLResObjectData::XMLResObjectData() : install_only(false)
      { }

      XMLPatternData::XMLPatternData() : userVisible(true)
      { }

      /*
      XMLPatchData::XMLPatchData()
      { }
      */

      /* output operators */

      namespace {
        /**
         * @short Generic stream output for lists of Ptrs
         * @param out the ostream where the output goes to
         * @param aList the list to output
         * @return is out
         */
        template<class T>
        ostream& operator<<(ostream &out, const list<T>& aList)
        {
          typedef typename list<T>::const_iterator IterType;
          for (IterType iter = aList.begin();
              iter != aList.end();
              ++iter) {
                if (iter != aList.begin())
                  out << endl;
                operator<<(out,*iter);
              }
          return out;
        }
      }

      /**
       * Join a list of strings into a single string
       * @param aList the list of strings
       * @param joiner what to put between the list elements
       * @return the joined string
       */
      string join(const list<string>& aList,
                  const string& joiner)
      {
        string res;
        for (list<string>::const_iterator iter = aList.begin();
            iter != aList.end();
            ++ iter) {
              if (iter != aList.begin())
                res += joiner;
              res += *iter;
            }
        return res;
      }

      ostream& operator<<(ostream &out, const XMLDependency& data)
      {
        if (! data.kind.empty())
          out << "[" << data.kind << "] ";
        out << data.encoded;
        return out;
      }

      ostream& operator<<(ostream &out, const XMLPatternData& data)
      {
        out << "-------------------------------------------------" << endl
          << "Pattern Data: " << endl
          << "name:" << data.name << endl
          << "summary: '" << data.summary << "'" << endl
          << "default: '" << data.default_  << "'" << endl
          << "user-visible: '" << data.userVisible  << "'" << endl
          << "description:" << endl << data.description << endl
	  << "category: " << data.category << endl
	  << "icon: " << data.icon << endl
	  << "script: " << data.script << endl
          << "provides:" << endl << data.provides << endl
          << "conflicts:" << endl << data.conflicts << endl
          << "obsoletes:" << endl << data.obsoletes << endl
          << "requires:" << endl << data.requires << endl
          << "recommends:" << endl << data.recommends << endl
          << "suggests:" << endl << data.suggests << endl
          << "supplements:" << endl << data.supplements << endl
          << "enhances:" << endl << data.enhances << endl
          << "freshens: " << endl << data.freshens << endl;
        return out;
      }

      ostream& operator<<(ostream &out, const XMLProductData& data)
      {
        out << "-------------------------------------------------" << endl
          << "Product Data: " << endl
          << "  type: " << data.type << endl
          << "  vendor: " << data.vendor << endl
          << "  name: " << data.name << endl
          << "  summary: " << data.summary << endl
          << "  description: " << data.description << endl
          << "  short name: " << data.short_name << endl
          << "  epoch: " << data.epoch << endl
          << "  version: " << data.ver << endl
          << "  release: " << data.rel << endl
          << "  dist_name: " << data.dist_name << endl
          << "  dist_version: " << data.dist_version << endl
          << "  provides: " << data.provides << endl
          << "  conflicts: " << data.conflicts << endl
          << "  obsoletes: " << data.obsoletes << endl
          << "  freshens: " << data.freshens << endl
          << "  requires: " << data.requires << endl
          << "  recommends:" << endl << data.recommends << endl
          << "  suggests:" << endl << data.suggests << endl
          << "  supplements:" << endl << data.supplements << endl
          << "  enhances:" << endl << data.enhances << endl;
        return out;
      }

      IMPL_PTR_TYPE(XMLResObjectData);
      IMPL_PTR_TYPE(XMLPatternData);
      IMPL_PTR_TYPE(XMLProductData);
      IMPL_PTR_TYPE(XMLLanguageData);
      IMPL_PTR_TYPE(XMLPatchData);
      IMPL_PTR_TYPE(XMLPatchAtomData);
      IMPL_PTR_TYPE(XMLPatchMessageData);
      IMPL_PTR_TYPE(XMLPatchScriptData);

    } // namespace xml
  } // namespace parser
} // namespace zypp
