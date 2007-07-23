/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/IniDict.h
 *
*/
#ifndef ZYPP_PARSER_INIDICT_H
#define ZYPP_PARSER_INIDICT_H

#include <iosfwd>
#include <map>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/Iterator.h"
#include "zypp/parser/IniParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : IniDict
    //
    /**
     * Parses a INI file and offers its structure as a
     * dictionary.
     * 
     */
    class IniDict : public IniParser
    {
      friend std::ostream & operator<<( std::ostream & str, const IniDict & obj );
    public:
      typedef std::map<std::string, std::string> EntrySet;
      typedef std::map<std::string, EntrySet> SectionSet;
      typedef MapKVIteratorTraits<SectionSet>::Key_const_iterator section_const_iterator;
      typedef EntrySet::const_iterator entry_const_iterator;
      
      /**
       * \name Section Iterators
       * Iterate trough ini file sections
       * \code
       * for ( IniDict::section_const_iterator it = dict.sectionsBegin(); 
       *       it != dict.sectionsEnd();
       *       ++it )
       * {
       *   MIL << (*it) << endl;
       * }
       * \endcode
       */
       //@{
      section_const_iterator sectionsBegin() const;
      section_const_iterator sectionsEnd() const;
      //@}
      
      /**
       * \name Entries Iterators
       * Iterate trough ini file entries in a section
       * \code
       * for ( IniDict::entry_const_iterator it = dict.entriesBegin("updates"); 
       *       it != dict.entriesEnd("updates");
       *       ++it )
       * {
       *   MIL << (*it).first << endl;
       * }
       * \endcode
       */
       
      //@{
      entry_const_iterator entriesBegin(const std::string &section) const;
      entry_const_iterator entriesEnd(const std::string &section) const;
      //@{
      
      /**
       * Creates a dictionary from a InputStream
       * containing a ini structured file
       */
      IniDict( const InputStream &is,
               const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );

      /**
       * Creates a mepty dictionary
       */
      IniDict();
      
      /** Dtor */
      ~IniDict();

      /**
       * Fill a dictionary from a InputStream
       * containing a ini structured file
       */
      void read( const InputStream &is,
                 const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );

      /**
       * \short add an entry
       * \param section
       * \param key
       * \param value
       */
      void insertEntry( const std::string &section,
                        const std::string &key,
                        const std::string &value );
      
      /**
       * \short add an entry
       * \param section
       * \param key
       * \param value
       */
      void deleteSection( const std::string &section );

      /**
       * \short True if there is a section with that name
       * \param section Section Name
       */
      bool hasSection( const std::string &section ) const;

      /**
       * \short True if an entry exists in the section
       * \param section Section name
       * \param entry entry name
       *
       * \note If the given section does not exist, this will
       * of course return false.
       */
      bool hasEntry( const std::string &section,
                     const std::string &entry ) const;
    public:

      /** Called when a section is found. */
      virtual void consume( const std::string &section );
      /** Called when a key value is found. */
      virtual void consume( const std::string &section,
                            const std::string &key,
                            const std::string &value );

    private:
      SectionSet _dict;
      /**
       * empty map used to simulate
       * iteration in non existant
       * sections
       */
      EntrySet _empty_map;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates IniDict Stream output */
    std::ostream & operator<<( std::ostream & str, const IniDict & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_INIDICT_H
