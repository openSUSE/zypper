/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMPatchParser.cc
 *
*/

#include <YUMPatchParser.h>
#include <YUMPrimaryParser.h>
#include <istream>
#include <string>
#include <cassert>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/parser/LibXMLHelper.h>
#include <zypp/base/Logger.h>
#include <schemanames.h>

using namespace std;
namespace zypp {
  namespace parser {
    namespace yum {

      YUMPatchParser::~YUMPatchParser()
      { }
      
      YUMPatchParser::YUMPatchParser(istream &is, const string& baseUrl)
      : XMLNodeIterator<YUMPatchDataPtr>(is, baseUrl,PATCHSCHEMA)
      {
        fetchNext();
      }
      
      YUMPatchParser::YUMPatchParser()
      { }
      
      YUMPatchParser::YUMPatchParser(YUMPatchDataPtr& entry)
      : XMLNodeIterator<YUMPatchDataPtr>(entry)
      { }
      
      
      // select for which elements process() will be called
      bool 
      YUMPatchParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "patch";
      }
      
      // do the actual processing
      YUMPatchDataPtr
      YUMPatchParser::process(const xmlTextReaderPtr reader)
      {
        assert(reader);
        YUMPatchDataPtr patchPtr = new YUMPatchData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        assert(dataNode);
        patchPtr->timestamp = _helper.attribute(dataNode,"timestamp");
        patchPtr->patchId = _helper.attribute(dataNode,"patchid");
        patchPtr->engine = _helper.attribute(dataNode,"engine");
      
        // default values for optional entries
        patchPtr->rebootNeeded = false;
        patchPtr->packageManager = false;
      
        // FIXME move the respective method to a common class, inherit it  
        YUMPrimaryParser prim;
      
        for (xmlNodePtr child = dataNode->children; 
             child && child != dataNode;
             child = child->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "name") {
      	patchPtr->name = _helper.content(child);
            }
            else if (name == "summary") {
              patchPtr->summary = MultiLang(
      	  _helper.attribute(child,"lang"),
      	  _helper.content(child));
            }
            else if (name == "description") {
              patchPtr->description = MultiLang(
      	  _helper.attribute(child,"lang"),
      	  _helper.content(child));
            }
            else if (name == "version") {
              patchPtr->epoch = _helper.attribute(child,"epoch");
              patchPtr->ver = _helper.attribute(child,"ver");
              patchPtr->rel = _helper.attribute(child,"rel");
            }
            else if (name == "provides") {
              prim.parseDependencyEntries(& patchPtr->provides, child);
            }
            else if (name == "conflicts") {
              prim.parseDependencyEntries(& patchPtr->conflicts, child);
            }
            else if (name == "obsoletes") {
              prim.parseDependencyEntries(& patchPtr->obsoletes, child);
            }
            else if (name == "requires") {
              prim.parseDependencyEntries(& patchPtr->requires, child);
            }
            else if (name == "freshen") {
              prim.parseDependencyEntries(& patchPtr->requires, child);
            }
            else if (name == "category") {
      	patchPtr->category = _helper.content(child);
            }
            else if (name == "reboot_needed") {
      	patchPtr->rebootNeeded = true;
            }
            else if (name == "package_manager") {
      	patchPtr->packageManager = true;
            }
            else if (name == "update_script") {
      	patchPtr->updateScript = _helper.content(child);
            }
            else if (name == "atoms") {
              parseAtomsNode(patchPtr, child);
            }
            else {
              WAR << "YUM <data> contains the unknown element <" << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
        return patchPtr;
      } /* end process */
      
      
      void 
      YUMPatchParser::parseAtomsNode(YUMPatchDataPtr dataPtr,
                                     xmlNodePtr formatNode)
      {
        assert(formatNode);
        for (xmlNodePtr child = formatNode->children; 
             child != 0;
             child = child ->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "package")
            {
      	parsePackageNode (dataPtr, child);
            }
            else if (name == "script")
            {
      	parseScriptNode (dataPtr, child);
            }
            else if (name == "message")
            {
      	parseMessageNode (dataPtr, child);
            }
            else {
              WAR << "YUM <atoms> contains the unknown element <" << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
      }
      
      void 
      YUMPatchParser::parseFormatNode(YUMPatchPackage *dataPtr,
                                              xmlNodePtr formatNode)
      {
        assert(formatNode);
        dataPtr->installOnly = false;
      
        // FIXME move the respective method to a common class, inherit it  
        YUMPrimaryParser prim;
      
        for (xmlNodePtr child = formatNode->children; 
             child != 0;
             child = child ->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "license") {
              dataPtr->license = _helper.content(child);
            }
            else if (name == "vendor") {
              dataPtr->vendor = _helper.content(child);
            }
            else if (name == "group") {
              dataPtr->group = _helper.content(child);
            }
            else if (name == "buildhost") {
              dataPtr->buildhost = _helper.content(child);
            }
            else if (name == "sourcerpm") {
              dataPtr->sourcerpm = _helper.content(child);
            }
            else if (name == "header-range") {
              dataPtr->headerStart = _helper.attribute(child,"start");
              dataPtr->headerEnd = _helper.attribute(child,"end");
            }
            else if (name == "provides") {
              prim.parseDependencyEntries(& dataPtr->provides, child);
            }
            else if (name == "conflicts") {
              prim.parseDependencyEntries(& dataPtr->conflicts, child);
            }
            else if (name == "obsoletes") {
              prim.parseDependencyEntries(& dataPtr->obsoletes, child);
            }
            else if (name == "requires") {
              prim.parseDependencyEntries(& dataPtr->requires, child);
            }
            else if (name == "file") {
              dataPtr->files.push_back
                (FileData(_helper.content(child),
                          _helper.attribute(child,"type")));
            }
            /* SUSE specific elements */
            else if (name == "authors") {
              prim.parseAuthorEntries(& dataPtr->authors, child);
            }
            else if (name == "keywords") {
              prim.parseKeywordEntries(& dataPtr->keywords, child);
            }
            else if (name == "media") {
              dataPtr->media = _helper.attribute(child,"mediaid");
            }
            else if (name == "dirsizes") {
              prim.parseDirsizeEntries(& dataPtr->dirSizes, child);
            }
            else if (name == "freshen") {
              prim.parseDependencyEntries(& dataPtr->freshen, child);
            }
            else if (name == "install_only") {
              dataPtr->installOnly = true;
            }
            else {
              WAR << "YUM <atom/package/format> contains the unknown element <"
      	  << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
      }
      
      void
      YUMPatchParser::parsePkgPlainRpmNode(YUMPatchPackage *dataPtr,
      				xmlNodePtr formatNode)
      {
        dataPtr->plainRpm.arch = _helper.attribute( formatNode, "arch" );
        dataPtr->plainRpm.filename = _helper.attribute( formatNode, "filename" );
        dataPtr->plainRpm.downloadsize = _helper.attribute( formatNode, "downloadsize" );
        dataPtr->plainRpm.md5sum = _helper.attribute( formatNode, "md5sum" );
        dataPtr->plainRpm.buildtime = _helper.attribute( formatNode, "buildtime" );
        for (xmlNodePtr child = formatNode->children; 
             child != 0;
             child = child ->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            WAR << "YUM <atom/package/pkgfiles/plain> contains the unknown element <"
      	<< name << "> "
              << _helper.positionInfo(child) << ", skipping" << endl;
          }
        }
      }
      
      void
      YUMPatchParser::parsePkgPatchRpmNode(YUMPatchPackage *dataPtr,
      				xmlNodePtr formatNode)
      {
        dataPtr->patchRpm.arch = _helper.attribute( formatNode, "arch" );
        dataPtr->patchRpm.filename = _helper.attribute( formatNode, "filename" );
        dataPtr->patchRpm.downloadsize = _helper.attribute( formatNode, "downloadsize" );
        dataPtr->patchRpm.md5sum = _helper.attribute( formatNode, "md5sum" );
        dataPtr->patchRpm.buildtime = _helper.attribute( formatNode, "buildtime" );
        for (xmlNodePtr child = formatNode->children; 
             child != 0;
             child = child ->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "base_version") {
      	YUMBaseVersion base_version;
      	parsePkgBaseVersionNode( &base_version, child);
              dataPtr->patchRpm.baseVersions.push_back( base_version );
            }
            else {
              WAR << "YUM <atom/package/pkgfiles/patch> contains the unknown element <"
      	  << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
      }
      
      void
      YUMPatchParser::parsePkgDeltaRpmNode(YUMPatchPackage *dataPtr,
      				xmlNodePtr formatNode)
      {
        dataPtr->deltaRpm.arch = _helper.attribute( formatNode, "arch" );
        dataPtr->deltaRpm.filename = _helper.attribute( formatNode, "filename" );
        dataPtr->deltaRpm.downloadsize = _helper.attribute( formatNode, "downloadsize" );
        dataPtr->deltaRpm.md5sum = _helper.attribute( formatNode, "md5sum" );
        dataPtr->deltaRpm.buildtime = _helper.attribute( formatNode, "buildtime" );
        for (xmlNodePtr child = formatNode->children; 
             child != 0;
             child = child ->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "base_version") {
      	parsePkgBaseVersionNode( &(dataPtr->deltaRpm.baseVersion), child);
            }
            else {
              WAR << "YUM <atom/package/pkgfiles/delta> contains the unknown element <"
      	  << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
      }
      
      
      void
      YUMPatchParser::parsePkgBaseVersionNode(YUMBaseVersion *dataPtr,
      					xmlNodePtr formatNode)
      {
        dataPtr->epoch = _helper.attribute( formatNode, "epoch" );
        dataPtr->ver = _helper.attribute( formatNode, "ver" );
        dataPtr->rel = _helper.attribute( formatNode, "rel" );
        dataPtr->md5sum = _helper.attribute( formatNode, "md5sum" );
        dataPtr->buildtime = _helper.attribute( formatNode, "buildtime" );
        dataPtr->source_info = _helper.attribute( formatNode, "source_info" );
      }
      
      void
      YUMPatchParser::parsePkgFilesNode(YUMPatchPackage *dataPtr,
      				 xmlNodePtr formatNode)
      {
        for (xmlNodePtr child = formatNode->children; 
             child != 0;
             child = child ->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "plainrpm") {
      	parsePkgPlainRpmNode( dataPtr, child );
            }
            else if (name == "patchrpm") {
      	parsePkgPatchRpmNode( dataPtr, child );
            }
            else if (name == "deltarpm") {
      	parsePkgDeltaRpmNode( dataPtr, child );
            }
            else {
              WAR << "YUM <atom/package/pkgfiles> contains the unknown element <"
      	  << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
      }
      
      
      
      
      
      
      void
      YUMPatchParser::parsePackageNode(YUMPatchDataPtr dataPtr,
                                     xmlNodePtr formatNode)
      {
        YUMPatchAtom at;
        at.type = "package";
        at.package = new YUMPatchPackage;
        at.package->type = _helper.attribute(formatNode,"type");
        at.package->installOnly = false;
      
        // FIXME move the respective method to a common class, inherit it  
        YUMPrimaryParser prim;
      
        for (xmlNodePtr child = formatNode->children; 
             child != 0;
             child = child ->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "name") {
      	at.package->name = _helper.content(child);
            }
            else if (name == "arch") {
              at.package->arch = _helper.content(child);
            }
            else if (name == "version") {
              at.package->epoch = _helper.attribute(child,"epoch");
              at.package->ver = _helper.attribute(child,"ver");
              at.package->rel = _helper.attribute(child,"rel");
            }
            else if (name == "checksum") {
              at.package->checksumType = _helper.attribute(child,"type");
              at.package->checksumPkgid = _helper.attribute(child,"pkgid");
              at.package->checksum = _helper.content(child);
            }
            else if (name == "summary") {
              at.package->summary = _helper.content(child);
            }
            else if (name == "description") {
              at.package->description = _helper.content(child);
            }
            else if (name == "packager") {
              at.package->packager = _helper.content(child);
            }
            else if (name == "url") {
              at.package->url = _helper.content(child);
            }
            else if (name == "time") {
              at.package->timeFile = _helper.attribute(child,"file");
              at.package->timeBuild = _helper.attribute(child,"build");
            }
            else if (name == "size") {
              at.package->sizePackage = _helper.attribute(child,"package");
              at.package->sizeInstalled = _helper.attribute(child,"installed");
              at.package->sizeArchive = _helper.attribute(child,"archive");
            }
            else if (name == "location") {
              at.package->location = _helper.attribute(child,"href");
            }
            else if (name == "format") {
      	parseFormatNode (&*at.package, child);
            }
            else if (name == "pkgfiles")
            {
      	parsePkgFilesNode (&*at.package, child);
            }
            else {
              WAR << "YUM <atoms/package> contains the unknown element <"
      	  << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
        dataPtr->atoms.push_back(at);
      }
      
      void
      YUMPatchParser::parseScriptNode(YUMPatchDataPtr dataPtr,
                                     xmlNodePtr formatNode)
      {
        YUMPatchAtom at;
        at.type = "script";
        at.script = new YUMPatchScript;
      
        // FIXME move the respective method to a common class, inherit it  
        YUMPrimaryParser prim;
      
        for (xmlNodePtr child = formatNode->children; 
             child != 0;
             child = child ->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "name") {
      	at.script->name = _helper.content(child);
            }
            else if (name == "version") {
              at.script->epoch = _helper.attribute(child,"epoch");
              at.script->ver = _helper.attribute(child,"ver");
              at.script->rel = _helper.attribute(child,"rel");
            }
            else if (name == "do") {
      	at.script->do_script = _helper.content(child);
            }
            else if (name == "undo") {
      	at.script->undo_script = _helper.content(child);
            }
            else if (name == "provides") {
              prim.parseDependencyEntries(& at.script->provides, child);
            }
            else if (name == "conflicts") {
              prim.parseDependencyEntries(& at.script->conflicts, child);
            }
            else if (name == "obsoletes") {
              prim.parseDependencyEntries(& at.script->obsoletes, child);
            }
            else if (name == "requires") {
              prim.parseDependencyEntries(& at.script->requires, child);
            }
            else if (name == "freshen") {
              prim.parseDependencyEntries(& at.script->requires, child);
            }
            else {
              WAR << "YUM <atoms/script> contains the unknown element <"
      	  << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
        dataPtr->atoms.push_back(at);
      }
      
      void
      YUMPatchParser::parseMessageNode(YUMPatchDataPtr dataPtr,
                                     xmlNodePtr formatNode)
      {
        YUMPatchAtom at;
        at.type = "message";
        at.message = new YUMPatchMessage;
        at.message->type = _helper.attribute(formatNode,"type");
      
        // FIXME move the respective method to a common class, inherit it  
        YUMPrimaryParser prim;
      
        for (xmlNodePtr child = formatNode->children; 
             child != 0;
             child = child ->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "name") {
      	at.message->name = _helper.content(child);
            }
            else if (name == "version") {
              at.message->epoch = _helper.attribute(child,"epoch");
              at.message->ver = _helper.attribute(child,"ver");
              at.message->rel = _helper.attribute(child,"rel");
            }
            else if (name == "text") {
      	at.message->text = _helper.content(child);
            }
            else if (name == "provides") {
              prim.parseDependencyEntries(& at.message->provides, child);
            }
            else if (name == "conflicts") {
              prim.parseDependencyEntries(& at.message->conflicts, child);
            }
            else if (name == "obsoletes") {
              prim.parseDependencyEntries(& at.message->obsoletes, child);
            }
            else if (name == "requires") {
              prim.parseDependencyEntries(& at.message->requires, child);
            }
            else if (name == "freshen") {
              prim.parseDependencyEntries(& at.message->requires, child);
            }
            else {
              WAR << "YUM <atoms/message> contains the unknown element <"
      	  << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
        dataPtr->atoms.push_back(at);
      }

    } // namespace yum
  } // namespace parser
} // namespace zypp
