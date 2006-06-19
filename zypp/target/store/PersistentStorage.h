/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/PersistentStorage.h
*
*/
#ifndef ZYPP_TARGET_PERSISTENTSTORAGE_H
#define ZYPP_TARGET_PERSISTENTSTORAGE_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include <zypp/Pathname.h>
#include <zypp/Url.h>
#include <zypp/Date.h>
#include <zypp/Patch.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PersistentStorage
    //
    /** */
    class PersistentStorage : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const PersistentStorage & obj );
      typedef intrusive_ptr<PersistentStorage> Ptr;
      typedef intrusive_ptr<const PersistentStorage> constPtr;
    public:
      /** Default ctor */
      PersistentStorage();
      /** Dtor */
      ~PersistentStorage();
      void doTest();

      struct SourceData
      {
        SourceData()
        {
          enabled = true;
          autorefresh = false;
        };

        bool enabled;
        bool autorefresh;
        Pathname product_dir;
        std::string type;
        Url url;
        Pathname cache_dir;
        std::string alias;
      };

    public:
      /**
       * Initializes the Storage when the system is located in some
       * root path. THIS MUST BE CALLED BEFORE DOING ANY OPERATION
       */
       void init(const Pathname &root);

      /**
       * true is backend was already initialized
       */
       bool isInitialized() const;

       /**
        * last modification
        */
       Date timestamp() const;
       
      /**
       * Stores a Resolvable in the active backend.
       */
      void storeObject( ResObject::constPtr resolvable );
      /**
       * Deletes a Resolvable from the active backend.
       */
      void deleteObject( ResObject::constPtr resolvable );
      /**
       * Query for installed Resolvables.
       */
      std::list<ResObject::Ptr> storedObjects() const;
       /**
       * Query for installed Resolvables of a certain kind.
       */
      std::list<ResObject::Ptr> storedObjects(const Resolvable::Kind kind) const;
       /**
       * Query for installed Resolvables of a certain kind by name
       * \a partial_match allows for text search.
       */
      std::list<ResObject::Ptr> storedObjects(const Resolvable::Kind kind, const std::string & name, bool partial_match = false) const;
      /////////////////////////////////////////////////////////
      // Resolvables Flags API
      ////////////////////////////////////////////////////////
      public:
      /**
       * Set a flag for a resolvable
       */
      void setObjectFlag( ResObject::constPtr resolvable, const std::string &flag );
      /**
       * Removes a flag for a resolvable
       */
      void removeObjectFlag( ResObject::constPtr resolvable, const std::string &flag );
      /**
       * Returns a set of flags a resolvable has stored
       */
      std::set<std::string> objectFlags( ResObject::constPtr resolvable ) const;
      /**
       * True if the resolvable has that flag
       */
      bool doesObjectHasFlag( ResObject::constPtr resolvable, const std::string &flag ) const;

      /////////////////////////////////////////////////////////
      // Named Flags API
      ////////////////////////////////////////////////////////
      public:
      void setFlag( const std::string &key, const std::string &flag );
      void removeFlag( const std::string &key, const std::string &flag );
      std::set<std::string> flags( const std::string &key ) const;
      bool hasFlag( const std::string &key, const std::string &flag ) const;

      /////////////////////////////////////////////////////////
      // SOURCES API
      ////////////////////////////////////////////////////////
      /**
       * Query for installed Sources
       */
      std::list<SourceData> storedSources() const;
      /**
       * Add a new installed source
       */
      void storeSource(const SourceData &data);
      /**
       * Delete an installed source
       */
      void deleteSource(const std::string &alias);

    private:
      class Private;
      shared_ptr<Private> d;
    };
    ///////////////////////////////////////////////////////////////////
    /** \relates PersistentStorage Stream output */
    std::ostream & operator<<( std::ostream & str, const PersistentStorage & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_PERSISTENTSTORAGE_H
