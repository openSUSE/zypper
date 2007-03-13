    //collectPkg(); 

    //---------------------------------------------------------------
    // drop packages not allowed for current architecture
/*
    if (!allowedArch (arch))
    {
 if ((splitted[3] == "src")      // found a src/nosrc package
     || (splitted[3] == "nosrc"))
 {
     string sharewith ((_tagset.getTagByIndex (SHAREWITH))->Data());
     pkgmaptype::iterator it = _pkgmap.find (sharewith);   // get binary for this source
     if ( it == _pkgmap.end() ) {
       // 2nd attempt: might be sharewith is .x86_64 but we're doing 32bit install.
       // Thus look for some N-V-R.*
       PkgName n( splitted[0].c_str() );
       for ( it = _pkgmap.begin(); it != _pkgmap.end(); ++it ) {
   if ( it->second.first
        && it->second.first->name() == n
        && it->second.first->version() ==  splitted[1]
        && it->second.first->release() ==  splitted[2] )
     break;
       }
     }
     if (it != _pkgmap.end() )
     {
   PMULPackageDataProviderPtr dataprovider = it->second.second;  // get dataprovider

   dataprovider->_attr_SOURCELOC = _tagset.getTagByIndex (LOCATION)->Pos();

         std::vector<std::string> sizesplit;
   stringutil::split (_tagset.getTagByIndex(SIZE)->Data(), sizesplit, " ", false);
   if (sizesplit.size() <= 0)
   {
       ERR << "No sizes for '" << single << "'" << endl;
   }
   else
   {
       dataprovider->_attr_SOURCESIZE = FSize (atoll(sizesplit[0].c_str()));
   }

     } // binary package to current source found

 } // this is a src/nosrc package
 return InstSrcError::E_ok;

    } // not allowed arch


    //---------------------------------------------------------------
    // Pkg -> PMPackage
    PkgName name (splitted[0].c_str());
    PkgEdition edition (splitted[1].c_str(), splitted[2].c_str());

    PMULPackageDataProviderPtr dataprovider ( new PMULPackageDataProvider (_source, pkgcache, localecache, ducache));
#warning STORE InstSrcPtr in DataProvider
    PMPackagePtr package (new PMPackage (name, edition, arch, dataprovider));

    //---------------------------------------------------------------
    // enter package to map for faster "=Shr:" (share) and packages.local lookup
    // must save dataprovider too since package does not allow access to it later :-(
    _pkgmap[single] = pair<PMPackagePtr, PMULPackageDataProviderPtr>(package, dataprovider);

    TaggedFile::Tag *tagptr; // for SET_MULTI()

#define SET_VALUE(tagname,value) \
    do { dataprovider->_attr_##tagname = value; } while (0)
#define GET_TAG(tagname) \
    _tagset.getTagByIndex (tagname)
#define SET_CACHE(tagname) \
    do { tagptr = GET_TAG (tagname); dataprovider->_attr_##tagname = tagptr->Pos(); } while (0)


    //---------------------------------------------------------------
    // pass PMSolvable data directly to instance

    std::list<std::string> pkglist;
    PMSolvable::PkgRelList_type rellist;

    if (pkgcache->retrieveData (GET_TAG(REQUIRES)->Pos(), pkglist))
    {
 rellist = PMSolvable::StringList2PkgRelList (pkglist);
 package->setRequires (rellist);
    }
    pkglist.clear();
    if (pkgcache->retrieveData (GET_TAG(PREREQUIRES)->Pos(), pkglist))
    {
 rellist = PMSolvable::StringList2PkgRelList (pkglist);
 package->addPreRequires (rellist); // rellist is modified after that
    }
    pkglist.clear();
    if (pkgcache->retrieveData (GET_TAG(PROVIDES)->Pos(), pkglist))
    {
 unsigned int before = pkglist.size();
 // filter splitprovides out
#warning Filtering splitprovides is not job of InstSrc! Solvable itself should handle this.
 for (std::list<std::string>::iterator it = pkglist.begin();
       it != pkglist.end(); )
 {
   PkgSplit testsplit( *it, true );
   if ( testsplit.valid() ) {
     dataprovider->_attr_SPLITPROVIDES.insert(testsplit);
     it = pkglist.erase (it);
   } else {
     ++it;
   }
 }
 if (dataprovider->_attr_SPLITPROVIDES.size () + pkglist.size() != before)
 {
     ERR << "*** LOST PROVIDES ***" << endl;
 }
 rellist = PMSolvable::StringList2PkgRelList (pkglist);
 package->setProvides (rellist);
    }
    pkglist.clear();
    if (pkgcache->retrieveData (GET_TAG(CONFLICTS)->Pos(), pkglist))
    {
 rellist = PMSolvable::StringList2PkgRelList (pkglist);
 package->setConflicts (rellist);
    }
    pkglist.clear();
    if (pkgcache->retrieveData (GET_TAG(OBSOLETES)->Pos(), pkglist))
    {
 rellist = PMSolvable::StringList2PkgRelList (pkglist);
 package->setObsoletes (rellist);
    }

    SET_CACHE (RECOMMENDS);
    SET_CACHE (SUGGESTS);

    //---------------------------------------------------------------
    // split =Loc: <medianr> <filename>
    // and adapt position for filename accordingly

    tagptr = GET_TAG (LOCATION);
    const char *location = tagptr->Data().c_str();
    const char *locationname = location;

    while (*locationname && isblank (*locationname)) locationname++;
    if ((dataprovider->_attr_MEDIANR = atoi (locationname)) == 0)
    {
 WAR << "** suspiciuous media nr '" << locationname << "'" << endl;
    }
    while (*locationname && isdigit (*locationname)) locationname++;
    while (*locationname && isblank (*locationname)) locationname++;
    if (*locationname)
    {
 dataprovider->_attr_LOCATION = TagRetrievalPos (tagptr->posDataStart() + (locationname-location), tagptr->posDataEnd());
    }
    else
    {
 ERR << "No location for " << package->name() << endl;
    }

    //---------------------------------------------------------------
    // SIZE

    stringutil::split ((GET_TAG(SIZE))->Data(), splitted, " ", false);
    if (splitted.size() <= 0)
    {
 ERR << "No archivesize for " << package->name() << endl;
    }
    else
    {
 SET_VALUE (ARCHIVESIZE, FSize (atoll(splitted[0].c_str())));
 if (splitted.size() < 2)
        {
     ERR << "No size for " << package->name() << endl;
 }
 else
 {
     SET_VALUE (SIZE, FSize (atoll(splitted[1].c_str())));
 }
    }
    SET_VALUE (BUILDTIME, Date (GET_TAG(BUILDTIME)->Data()));

    SET_VALUE (GROUP, Y2PM::packageManager().addRpmGroup (GET_TAG(GROUP)->Data()));
    SET_CACHE (LICENSE);
    SET_CACHE (AUTHORS);
    SET_CACHE (KEYWORDS);

#undef SET_VALUE
#undef SET_POS
#undef GET_TAG
#undef SET_CACHE

    //---------------------------------------------------------------
    // SHAREWITH, package to share data with
    // FIXME: does not support forwared shared declarations

    string sharewith ((_tagset.getTagByIndex (SHAREWITH))->Data());
    if (!sharewith.empty())
    {  pkgpos = _pkgmap.find (sharewith);
 if (pkgpos == _pkgmap.end())
 {
     WAR << "Share package '" << sharewith << "' not found" << endl;
 }
 else
 {
     // Pass PMSolvable data to current package since these values
     // are *not* covered by the dataprovider fallback mechanism
     // for shared packages

     // *it ==   <std::string, std::pair<PMPackagePtr, PMULPackageDataProviderPtr> >

     PMPackagePtr share_target = pkgpos->second.first;

     if (package->requires().size() == 0)    // not own requires
   package->setRequires (share_target->requires());
     if (package->prerequires().size() == 0)
     {
   PMSolvable::PkgRelList_type prereqs = share_target->prerequires();
   package->addPreRequires (prereqs);
     }
     if (package->provides().size() == 0)
   package->setProvides (share_target->provides());
     if (package->conflicts().size() == 0)
   package->setConflicts (share_target->conflicts());
     if (package->obsoletes().size() == 0)
   package->setObsoletes (share_target->obsoletes());

     // rellist is modified after that
     // tell my dataprovider to share data with another dataprovider
     // all data not present in my dataprovider will be taken
     // from this dataprovider
     dataprovider->setShared ( pkgpos->second.second );
 }
    }

    return InstSrcError::E_ok;
  */
}

