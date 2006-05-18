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

#include <zypp/parser/yum/YUMPatchParser.h>
#include <zypp/parser/yum/YUMPrimaryParser.h>
#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/parser/LibXMLHelper.h>
#include <zypp/base/Logger.h>
#include <zypp/parser/yum/schemanames.h>
#include <zypp/ZYppFactory.h>

using namespace std;
namespace zypp {
  namespace parser {
    namespace yum {

      YUMPatchParser::~YUMPatchParser()
      { }
      
      YUMPatchParser::YUMPatchParser(istream &is, const string& baseUrl)
      : XMLNodeIterator<YUMPatchData_Ptr>(is, baseUrl,PATCHSCHEMA)
	, _zypp_architecture( getZYpp()->architecture() )
      {
	fetchNext();
      }
      
      YUMPatchParser::YUMPatchParser()
	: _zypp_architecture( getZYpp()->architecture() )
      { }
      
      YUMPatchParser::YUMPatchParser(YUMPatchData_Ptr& entry)
      : XMLNodeIterator<YUMPatchData_Ptr>(entry)
	, _zypp_architecture( getZYpp()->architecture() )
      { }
      
      
      // select for which elements process() will be called
      bool 
      YUMPatchParser::isInterested(const xmlNodePtr nodePtr)
      {
	return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "patch";
      }
      
      // do the actual processing
      YUMPatchData_Ptr
      YUMPatchParser::process(const xmlTextReaderPtr reader)
      {
	xml_assert(reader);
	YUMPatchData_Ptr patchPtr = new YUMPatchData;
	xmlNodePtr dataNode = xmlTextReaderExpand(reader);
	xml_assert(dataNode);
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
	    else if (name == "arch") {
		patchPtr->arch = _helper.content(child);
	      try {
		if (!Arch(patchPtr->arch).compatibleWith( _zypp_architecture )) {
		    patchPtr = NULL;			// skip <patch>, incompatible architecture
		    break;
		}
	      }
	      catch( const Exception & excpt_r ) {
		ZYPP_CAUGHT( excpt_r );
		DBG << "Skipping malformed " << patchPtr->arch << endl;
		patchPtr = NULL;
		break;
	      }
	    }
	    else if (name == "summary") {
	      patchPtr->summary.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
	    }
	    else if (name == "description") {
	      patchPtr->description.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
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
	    else if (name == "recommends") {
	      prim.parseDependencyEntries(& patchPtr->recommends, child);
	    }
	    else if (name == "suggests") {
	      prim.parseDependencyEntries(& patchPtr->suggests, child);
	    }
	    else if (name == "supplements") {
	      prim.parseDependencyEntries(& patchPtr->supplements, child);
	    }
	    else if (name == "enhances") {
	      prim.parseDependencyEntries(& patchPtr->enhances, child);
	    }
	    else if (name == "freshens") {
	      prim.parseDependencyEntries(& patchPtr->freshens, child);
	    }
	    else if (name == "category") {
		patchPtr->category = _helper.content(child);
	    }
	    else if (name == "reboot-needed") {
		patchPtr->rebootNeeded = true;
	    }
	    else if (name == "package-manager") {
		patchPtr->packageManager = true;
	    }
	    else if (name == "update-script") {
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
      YUMPatchParser::parseAtomsNode(YUMPatchData_Ptr dataPtr,
		                     xmlNodePtr formatNode)
      {
	xml_assert(formatNode);
	for (xmlNodePtr child = formatNode->children; 
	     child != 0;
	     child = child ->next) {
	  if (_helper.isElement(child)) {
	    string name = _helper.name(child);
XXX << "parseAtomsNode(" << name << ")" << endl;
	    if (name == "package")
	    {
		parsePackageNode( dataPtr, child );
	    }
	    else if (name == "script")
	    {
		parseScriptNode( dataPtr, child );
	    }
	    else if (name == "message")
	    {
		parseMessageNode( dataPtr, child );
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
	xml_assert(formatNode);
	dataPtr->installOnly = false;
	dataPtr->media = "1";
      
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
	    else if (name == "recommends") {
	      prim.parseDependencyEntries(& dataPtr->recommends, child);
	    }
	    else if (name == "suggests") {
	      prim.parseDependencyEntries(& dataPtr->suggests, child);
	    }
	    else if (name == "supplements") {
	      prim.parseDependencyEntries(& dataPtr->supplements, child);
	    }
	    else if (name == "enhances") {
	      prim.parseDependencyEntries(& dataPtr->enhances, child);
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
	    else if (name == "freshens") {
	      prim.parseDependencyEntries(& dataPtr->freshens, child);
	    }
	    else if (name == "install-only") {
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
	YUMPlainRpm plainRpm;
	plainRpm.arch = _helper.attribute( formatNode, "arch" );
	plainRpm.filename = _helper.attribute( formatNode, "filename" );
	plainRpm.downloadsize = _helper.attribute( formatNode, "downloadsize" );
	plainRpm.md5sum = _helper.attribute( formatNode, "md5sum" );
	plainRpm.buildtime = _helper.attribute( formatNode, "buildtime" );
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
	dataPtr->plainRpms.push_back(plainRpm);
      }
      
      void
      YUMPatchParser::parsePkgPatchRpmNode(YUMPatchPackage *dataPtr,
					xmlNodePtr formatNode)
      {
	YUMPatchRpm patchRpm;
	patchRpm.arch = _helper.attribute( formatNode, "arch" );
	patchRpm.filename = _helper.attribute( formatNode, "filename" );
	patchRpm.downloadsize = _helper.attribute( formatNode, "downloadsize" );
	patchRpm.md5sum = _helper.attribute( formatNode, "md5sum" );
	patchRpm.buildtime = _helper.attribute( formatNode, "buildtime" );
	for (xmlNodePtr child = formatNode->children; 
	     child != 0;
	     child = child ->next) {
	  if (_helper.isElement(child)) {
	    string name = _helper.name(child);
	    if (name == "base-version") {
	      YUMBaseVersion base_version;
	      parsePkgBaseVersionNode( &base_version, child);
	      patchRpm.baseVersions.push_back( base_version );
	    }
            else if (name == "location") {
		patchRpm.location = _helper.attribute(child,"href");
	    }
	    else if (name == "media") {
		patchRpm.media = _helper.content(child);
	    }
	    else if (name == "checksum") {
	      patchRpm.checksumType = _helper.attribute(child,"type");
	      patchRpm.checksum = _helper.content(child);
	    }
            else if (name == "time") {
//              patchRpm.timeFile = _helper.attribute(child,"file");
              patchRpm.buildtime = _helper.attribute(child,"build");
            }
            else if (name == "size") {
//              patchRpm.sizePackage = _helper.attribute(child,"package");
//              patchRpm.sizeInstalled = _helper.attribute(child,"installed");
              patchRpm.downloadsize = _helper.attribute(child,"archive");
            }
	    else {
	      WAR << "YUM <atom/package/pkgfiles/patch> contains the unknown element <"
		  << name << "> "
		<< _helper.positionInfo(child) << ", skipping" << endl;
	    }
	  }
	}
	dataPtr->patchRpms.push_back(patchRpm);
      }
      
      void
      YUMPatchParser::parsePkgDeltaRpmNode(YUMPatchPackage *dataPtr,
					xmlNodePtr formatNode)
      {
	YUMDeltaRpm deltaRpm;
	deltaRpm.arch = _helper.attribute( formatNode, "arch" );
	deltaRpm.filename = _helper.attribute( formatNode, "filename" );
	deltaRpm.downloadsize = _helper.attribute( formatNode, "downloadsize" );
	deltaRpm.md5sum = _helper.attribute( formatNode, "md5sum" );
	deltaRpm.buildtime = _helper.attribute( formatNode, "buildtime" );
	for (xmlNodePtr child = formatNode->children; 
	     child != 0;
	     child = child ->next) {
	  if (_helper.isElement(child)) {
	    string name = _helper.name(child);
	    if (name == "base-version") {
	      parsePkgBaseVersionNode( &(deltaRpm.baseVersion), child);
	    }
            else if (name == "location") {
		deltaRpm.location = _helper.attribute(child,"href");
	    }
	    else if (name == "media") {
		deltaRpm.media = _helper.content(child);
	    }
	    else if (name == "checksum") {
	      deltaRpm.checksumType = _helper.attribute(child,"type");
	      deltaRpm.checksum = _helper.content(child);
	    }
            else if (name == "time") {
//              deltaRpm.timeFile = _helper.attribute(child,"file");
              deltaRpm.buildtime = _helper.attribute(child,"build");
            }
            else if (name == "size") {
//              deltaRpm.sizePackage = _helper.attribute(child,"package");
//              deltaRpm.sizeInstalled = _helper.attribute(child,"installed");
              deltaRpm.downloadsize = _helper.attribute(child,"archive");
            }
	    else {
	      WAR << "YUM <atom/package/pkgfiles/delta> contains the unknown element <"
		  << name << "> "
		<< _helper.positionInfo(child) << ", skipping" << endl;
	    }
	  }
	}
	dataPtr->deltaRpms.push_back(deltaRpm);
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
	dataPtr->source_info = _helper.attribute( formatNode, "source-info" );
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
      YUMPatchParser::parsePackageNode(YUMPatchData_Ptr dataPtr,
		                     xmlNodePtr formatNode)
      {
	shared_ptr<YUMPatchPackage> package(new YUMPatchPackage);
	package->type = _helper.attribute(formatNode,"type");
	package->installOnly = false;
	package->media = "1";
      
	// FIXME move the respective method to a common class, inherit it  
	YUMPrimaryParser prim;
      
	for (xmlNodePtr child = formatNode->children; 
	     child != 0;
	     child = child ->next) {
	  if (_helper.isElement(child)) {
	    string name = _helper.name(child);
XXX << "parsePackageNode(" << name << ")" << endl;
	    if (name == "name") {
		package->name = _helper.content(child);
	    }
	    else if (name == "arch") {
	      package->arch = _helper.content(child);
	    }
	    else if (name == "version") {
	      package->epoch = _helper.attribute(child,"epoch");
	      package->ver = _helper.attribute(child,"ver");
	      package->rel = _helper.attribute(child,"rel");
	    }
	    else if (name == "checksum") {
	      package->checksumType = _helper.attribute(child,"type");
	      package->checksumPkgid = _helper.attribute(child,"pkgid");
	      package->checksum = _helper.content(child);
	    }
	    else if (name == "summary") {
	      package->summary = _helper.content(child);
	    }
	    else if (name == "description") {
	      package->description = _helper.content(child);
	    }
	    else if (name == "packager") {
	      package->packager = _helper.content(child);
	    }
	    else if (name == "url") {
	      package->url = _helper.content(child);
	    }
	    else if (name == "time") {
	      package->timeFile = _helper.attribute(child,"file");
	      package->timeBuild = _helper.attribute(child,"build");
	    }
	    else if (name == "size") {
	      package->sizePackage = _helper.attribute(child,"package");
	      package->sizeInstalled = _helper.attribute(child,"installed");
	      package->sizeArchive = _helper.attribute(child,"archive");
	    }
	    else if (name == "location") {
	      package->location = _helper.attribute(child,"href");
	    }
	    else if (name == "format") {
		parseFormatNode (&*package, child);
	    }
	    else if (name == "pkgfiles")
	    {
		parsePkgFilesNode (&*package, child);
	    }
	    else if (name == "license-to-confirm")
	    {
	      package->license_to_confirm.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
	    }
	    else {
	      WAR << "YUM <atoms/package> contains the unknown element <"
		  << name << "> "
		<< _helper.positionInfo(child) << ", skipping" << endl;
	    }
	  }
	}
	dataPtr->atoms.push_back(package);
      }
      
      void
      YUMPatchParser::parseScriptNode(YUMPatchData_Ptr dataPtr,
		                     xmlNodePtr formatNode)
      {
	shared_ptr<YUMPatchScript> script(new YUMPatchScript);
	script->do_media = "1";
	script->undo_media = "1";
      
	// FIXME move the respective method to a common class, inherit it  
	YUMPrimaryParser prim;
      
	for (xmlNodePtr child = formatNode->children; 
	     child != 0;
	     child = child ->next) {
	  if (_helper.isElement(child)) {
	    string name = _helper.name(child);
	    if (name == "name") {
		script->name = _helper.content(child);
	    }
	    else if (name == "version") {
	      script->epoch = _helper.attribute(child,"epoch");
	      script->ver = _helper.attribute(child,"ver");
	      script->rel = _helper.attribute(child,"rel");
	    }
	    else if (name == "do") {
		script->do_script = _helper.content(child);
	    }
	    else if (name == "undo") {
		script->undo_script = _helper.content(child);
	    }
            else if (name == "do-location") {
		script->do_location = _helper.attribute(child,"href");
	    }
            else if (name == "undo-location") {
		script->undo_location = _helper.attribute(child,"href");
	    }
	    else if (name == "do-media") {
		script->do_media = _helper.attribute(child,"mediaid");
	    }
	    else if (name == "undo-media") {
		script->undo_media = _helper.attribute(child,"mediaid");
	    }
	    else if (name == "do-checksum") {
	      script->do_checksum_type = _helper.attribute(child,"type");
	      script->do_checksum = _helper.content(child);
	    }
	    else if (name == "undo-checksum") {
	      script->undo_checksum_type = _helper.attribute(child,"type");
	      script->undo_checksum = _helper.content(child);
	    }
	    else if (name == "provides") {
	      prim.parseDependencyEntries(& script->provides, child);
	    }
	    else if (name == "conflicts") {
	      prim.parseDependencyEntries(& script->conflicts, child);
	    }
	    else if (name == "obsoletes") {
	      prim.parseDependencyEntries(& script->obsoletes, child);
	    }
	    else if (name == "requires") {
	      prim.parseDependencyEntries(& script->requires, child);
	    }
	    else if (name == "recommends") {
	      prim.parseDependencyEntries(& script->recommends, child);
	    }
	    else if (name == "suggests") {
	      prim.parseDependencyEntries(& script->suggests, child);
	    }
	    else if (name == "supplements") {
	      prim.parseDependencyEntries(& script->supplements, child);
	    }
	    else if (name == "enhances") {
	      prim.parseDependencyEntries(& script->enhances, child);
	    }
	    else if (name == "freshens") {
	      prim.parseDependencyEntries(& script->freshens, child);
	    }
	    else {
	      WAR << "YUM <atoms/script> contains the unknown element <"
		  << name << "> "
		<< _helper.positionInfo(child) << ", skipping" << endl;
	    }
	  }
	}
	dataPtr->atoms.push_back(script);
      }
      
      void
      YUMPatchParser::parseMessageNode(YUMPatchData_Ptr dataPtr,
		                     xmlNodePtr formatNode)
      {
	shared_ptr<YUMPatchMessage> message(new YUMPatchMessage);
      
	// FIXME move the respective method to a common class, inherit it  
	YUMPrimaryParser prim;
      
	for (xmlNodePtr child = formatNode->children; 
	     child != 0;
	     child = child ->next) {
	  if (_helper.isElement(child)) {
	    string name = _helper.name(child);
	    if (name == "name") {
	      message->name = _helper.content(child);
	    }
	    else if (name == "version") {
	      message->epoch = _helper.attribute(child,"epoch");
	      message->ver = _helper.attribute(child,"ver");
	      message->rel = _helper.attribute(child,"rel");
	    }
	    else if (name == "text") {
	      message->text.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
	    }
	    else if (name == "provides") {
	      prim.parseDependencyEntries(& message->provides, child);
	    }
	    else if (name == "conflicts") {
	      prim.parseDependencyEntries(& message->conflicts, child);
	    }
	    else if (name == "obsoletes") {
	      prim.parseDependencyEntries(& message->obsoletes, child);
	    }
	    else if (name == "requires") {
	      prim.parseDependencyEntries(& message->requires, child);
	    }
	    else if (name == "recommends") {
	      prim.parseDependencyEntries(& message->recommends, child);
	    }
	    else if (name == "suggests") {
	      prim.parseDependencyEntries(& message->suggests, child);
	    }
	    else if (name == "supplements") {
	      prim.parseDependencyEntries(& message->supplements, child);
	    }
	    else if (name == "enhances") {
	      prim.parseDependencyEntries(& message->enhances, child);
	    }
	    else if (name == "freshens") {
	      prim.parseDependencyEntries(& message->freshens, child);
	    }
	    else {
	      WAR << "YUM <atoms/message> contains the unknown element <"
		  << name << "> "
		<< _helper.positionInfo(child) << ", skipping" << endl;
	    }
	  }
	}
	dataPtr->atoms.push_back(message);
      }

    } // namespace yum
  } // namespace parser
} // namespace zypp
