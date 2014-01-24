/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/FileConflicts.h
 */
#ifndef ZYPP_SAT_FILECONFLICTS_H
#define ZYPP_SAT_FILECONFLICTS_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/sat/Queue.h"
#include "zypp/sat/Solvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    ///////////////////////////////////////////////////////////////////
    /// \class FileConflicts
    /// \brief Libsolv queue representing file conflicts.
    ///////////////////////////////////////////////////////////////////
    class FileConflicts : private Queue
    {
      friend bool operator==( const FileConflicts & lhs, const FileConflicts & rhs );
      static constexpr size_type queueBlockSize = 6;

    public:
      /**
       * \class Conflict
       * \brief A file conflict.
       */
      struct Conflict
      {
	IdString lhsFilename() const	{ return IdString( _data[0] ); }
	Solvable lhsSolvable() const	{ return Solvable( _data[1] ); }
	IdString lhsFilemd5() const	{ return IdString( _data[2] ); }

	IdString rhsFilename() const	{ return IdString( _data[3] ); }
	Solvable rhsSolvable() const	{ return Solvable( _data[4] ); }
	IdString rhsFilemd5() const	{ return IdString( _data[5] ); }

	/** Ready to use (translated) string describing the Conflict */
	std::string asUserString() const;

      private:
	detail::IdType _data[queueBlockSize];
      };

    public:
      using Queue::size_type;
      typedef Conflict value_type;
      typedef const value_type* const_iterator;

      using Queue::empty;
      size_type size() const		{ return Queue::size()/queueBlockSize; }
      const_iterator begin() const	{ return reinterpret_cast<const_iterator>(Queue::begin()); }
      const_iterator end() const	{ return reinterpret_cast<const_iterator>(Queue::end()); }

    public:
      using Queue::operator struct ::_Queue *;		///< libsolv backdoor
      using Queue::operator const struct ::_Queue *;	///< libsolv backdoor
    };

    /** \relates FileConflicts Stream output */
    std::ostream & operator<<( std::ostream & str, const FileConflicts & obj );

    /** \relates FileConflicts::Conflict Stream output */
    std::ostream & operator<<( std::ostream & str, const FileConflicts::Conflict & obj );

    /** \relates FileConflicts XML output */
    std::ostream & dumpAsXMLOn( std::ostream & str, const FileConflicts & obj );

    /** \relates FileConflicts::Conflict XML output */
    std::ostream & dumpAsXMLOn( std::ostream & str, const FileConflicts::Conflict & obj );

    /** \relates FileConflicts */
    inline bool operator==( const FileConflicts & lhs, const FileConflicts & rhs )
    { return static_cast<const Queue &>(lhs) == static_cast<const Queue &>(rhs); }

    /** \relates FileConflicts */
    inline bool operator!=( const FileConflicts & lhs, const FileConflicts & rhs )
    { return !( lhs == rhs ); }

  } // namespace sat
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_FILECONFLICTS_H
