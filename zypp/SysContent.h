/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SysContent.h
 *
*/
#ifndef ZYPP_SYSCONTENT_H
#define ZYPP_SYSCONTENT_H

#include <iosfwd>
#include <string>
#include <set>

#include "zypp/base/PtrTypes.h"

#include "zypp/PoolItem.h"
#include "zypp/Edition.h"
#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace syscontent
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Writer
    //
    /** Collect and serialize a set of \ref ResObject.
     * \code
     * <?xml version="1.0" encoding="UTF-8"?>
     * <syscontent>
     *   <ident>
     *     <name>mycollection</name>
     *     <version epoch="0" ver="1.0" rel="1"/>
     *     <description>All the cool stuff...</description>
     *     <created>1165270942</created>
     *   </ident>
     *   <onsys>
     *     <entry kind="package" name="pax" epoch="0" ver="3.4" rel="12" arch="x86_64"/>
     *     <entry kind="product" name="SUSE_SLES" epoch="0" ver="10" arch="x86_64"/>
     *     <entry ...
     *   </onsys>
     * </syscontent>
     * \endcode
     * \see Reader
    */
    class Writer
    {
      typedef std::set<ResObject::constPtr> StorageT;
    public:
      typedef StorageT::value_type     value_type;
      typedef StorageT::size_type      size_type;
      typedef StorageT::iterator       iterator;
      typedef StorageT::const_iterator const_iterator;

    public:
      /** Default Ctor. */
      Writer();

    public:
      /** \name Identification.
       * User provided optional data to identify the collection.
      */
      //@{
      /** Get name. */
      const std::string & name() const;

      /** Set name. */
      Writer & name( const std::string & val_r );

      /** Get edition. */
      const Edition & edition() const;

      /** Set edition. */
      Writer & edition( const Edition & val_r );

      /** Get description. */
      const std::string & description() const;

      /** Set description.*/
      Writer & description( const std::string & val_r );
      //@}

    public:
      /** \name Collecting data.
       * \code
       * syscontent::Writer contentW;
       * contentW.name( "mycollection" )
       *         .edition( Edition( "1.0" ) )
       *         .description( "All the cool stuff..." );
       *
       * ResPool pool( getZYpp()->pool() );
       * for_each( pool.begin(), pool.end(),
       *           bind( &syscontent::Writer::addIf, ref(contentW), _1 ) );
       *
       * std::ofstream my_file( "some_file" );
       * my_file << contentW;
       * my_file.close();
       * \endcode
      */
      //@{
      /** Collect currently installed \ref PoolItem. */
      void addInstalled( const PoolItem & obj_r );

      /** Collect \ref PoolItem if it stays on the system.
       * I.e. it stays installed or is tagged to be installed.
       * Solver selected items are omitted.
      */
      void addIf( const PoolItem & obj_r );

      /** Unconditionally add this \ref ResObject (or \ref PoolItem). */
      void add( const ResObject::constPtr & obj_r );
      //@}

    public:
      /** \name Collected data. */
      //@{
      /** Whether no data collected so far. */
      bool empty() const;

      /** Number of items collected. */
      size_type size() const;

      /** Iterator to the begin of collected data. */
      const_iterator begin() const;

      /** Iterator to the end of collected data. */
      const_iterator end() const;
      //@}

    public:
      /** Write collected data as XML.
       * Read them back using \ref Reader.
      */
      std::ostream & writeXml( std::ostream & str ) const;

    private:
      class Impl;
      RWCOW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Writer Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Writer & obj )
    { return obj.writeXml( str ); }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Reader
    //
    /** Retrieve \ref ResObject data serialized by \ref Writer.
     * \see Writer
    */
    class Reader
    {
    public:
      /** Restored \ref ResObject data. */
      class Entry;

    private:
      typedef std::list<Entry> StorageT;

    public:
      typedef StorageT::value_type     value_type;
      typedef StorageT::size_type      size_type;
      typedef StorageT::iterator       iterator;
      typedef StorageT::const_iterator const_iterator;

    public:
      /** Default Ctor. */
      Reader();

      /** Ctor parsing data from \a input_r.
       * \throws Exception on read or parse error.
      */
      Reader( std::istream & input_r );

    public:
      /** \name Identification.
       * User provided optional data to identify the collection.
      */
      //@{
      /** Get name. */
      const std::string & name() const;

      /** Get edition. */
      const Edition & edition() const;

      /** Get description. */
      const std::string & description() const;

      /** Get creation date. */
      const Date & ctime() const;

    public:
      /** \name Collected data. */
      //@{
      /** Whether no data collected so far. */
      bool empty() const;

      /** Number of items collected. */
      size_type size() const;

      /** Iterator to the begin of collected data. */
      const_iterator begin() const;

      /** Iterator to the end of collected data. */
      const_iterator end() const;
      //@}

    private:
      class Impl;
      RWCOW_pointer<Impl> _pimpl;
    };

    /** \relates Reader Stream output */
    std::ostream & operator<<( std::ostream & str, const Reader & obj );

    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Reader::Entry
    //
    /** Restored \ref ResObject data. */
    class Reader::Entry
    {
    public:
      Entry();
      const std::string & kind() const;
      const std::string & name() const;
      const Edition & edition() const;
      const Arch & arch() const;
    public:
      class Impl;
      Entry( const shared_ptr<Impl> & pimpl_r );
    private:
      RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace syscontent
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SYSCONTENT_H
