/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

/** \file zypp/HistoryLogData.h
 *
 */
#ifndef ZYPP_HISTORYLOGDATA_H_
#define ZYPP_HISTORYLOGDATA_H_

#include <iosfwd>

#include "zypp/APIConfig.h"
#include "zypp/Date.h"
#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/CheckSum.h"
#include "zypp/Url.h"

#define HISTORY_LOG_DATE_FORMAT "%Y-%m-%d %H:%M:%S"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class HistoryActionID
  /// \brief Enumeration of known history actions.
  /// \ingroup g_EnumerationClass
  /// \ingroup g_ZyppHistory
  ///////////////////////////////////////////////////////////////////
  struct HistoryActionID
  {
    static const HistoryActionID NONE;

    static const HistoryActionID INSTALL;
    static const HistoryActionID REMOVE;
    static const HistoryActionID REPO_ADD;
    static const HistoryActionID REPO_REMOVE;
    static const HistoryActionID REPO_CHANGE_ALIAS;
    static const HistoryActionID REPO_CHANGE_URL;

    enum ID
    {
      NONE_e,

      INSTALL_e,
      REMOVE_e,
      REPO_ADD_e,
      REPO_REMOVE_e,
      REPO_CHANGE_ALIAS_e,
      REPO_CHANGE_URL_e
    };

    HistoryActionID() : _id(NONE_e) {}

    HistoryActionID(ID id) : _id(id) {}

    explicit HistoryActionID(const std::string & strval_r);

    ID toEnum() const { return _id; }

    static HistoryActionID::ID parse(const std::string & strval_r);

    const std::string & asString(bool pad = false) const;

    private:
    ID _id;
  };

  /** \relates HistoryActionID */
  inline bool operator==( const HistoryActionID & lhs, const HistoryActionID & rhs )
  { return lhs.toEnum() == rhs.toEnum(); }

  /** \relates HistoryActionID */
  inline bool operator!=( const HistoryActionID & lhs, const HistoryActionID & rhs )
  { return lhs.toEnum() != rhs.toEnum(); }

  /** \relates HistoryActionID */
  std::ostream & operator << (std::ostream & str, const HistoryActionID & id);
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class HistoryLogData
  /// \brief A zypp history log line split into fields
  /// \ingroup g_ZyppHistory
  ///
  /// Each valid history log line starts with a date and HistoryActionID
  /// field. Subsequent fields depend on the kind of action. See derived
  /// classes for convenient access to those flields.
  ///
  /// HistoryLogData itself provides mostly generic access to the fields
  /// plain string values. Derived classes for well known entries tell
  ///
  ///////////////////////////////////////////////////////////////////
  class HistoryLogData
  {
  public:
    typedef shared_ptr<HistoryLogData>		Ptr;
    typedef shared_ptr<const HistoryLogData>	constPtr;

    typedef std::vector<std::string>	FieldVector;
    typedef FieldVector::size_type 	size_type;
    typedef FieldVector::const_iterator	const_iterator;

  public:
    /** Ctor \b moving \a FieldVector (via swap).
     * \throws ParseException if \a fields_r has not at least \a expect_r entries
     * \note 2 fields (date and action) are always required.
     */
    explicit HistoryLogData( FieldVector & fields_r, size_type expect_r = 2 );

    /** Ctor \b moving \a FieldVector (via swap).
     * \throws ParseException if \a fields_r has the wrong \ref HistoryActionID or not at least \a expect_r entries.
     * \note 2 fields (date and action) are always required.
     */
    HistoryLogData( FieldVector & fields_r, HistoryActionID action_r, size_type expect_r = 2 );

    /** Dtor */
    virtual ~HistoryLogData();

    /** Factory method creating HistoryLogData classes.
     *
     * Moves \a fields_r into a HistoryLogData or derived object, depending on the
     * HistoryActionID. For known action ids a coresponing HistoryLogData class
     * is created, to allow convenient access to the field values. For unknown
     * action ids a plain HistoryLogData object is created. \ref HistoryActionID
     * \ref NONE_e id used in this case.
     *
     * \throws ParseException if \a fields_r does not contain the required format.
     */
    static Ptr create( FieldVector & fields_r );

  public:
    /** Whether FieldVector is empty. */
    bool empty() const;

    /** Number of fields in vector. */
    size_type size() const;

    /** Iterator pointing to 1st element in vector (or end()). */
    const_iterator begin() const;

    /** Iterator pointing behind the last element in vector. */
    const_iterator end() const;

    /** Access (optional) field by number.
     * \returns an empty string if \a idx_r is out of range.
     * \see \ref at
     */
    const std::string & optionalAt( size_type idx_r ) const;
    /** \overload */
    const std::string & operator[]( size_type idx_r ) const
    { return optionalAt( idx_r ); }

    /** Access (required) field by number.
     * \throws std::out_of_range if \a idx_r is out of range.
     * \see \ref optionalAt
     */
    const std::string & at( size_type idx_r ) const;

  public:
    enum Index			///< indices of known fields
    {
      DATE_INDEX	= 0,	///< date
      ACTION_INDEX	= 1,	///< HistoryActionID
    };

  public:
    Date	date()		const;	///< date
    HistoryActionID action()	const;	///< HistoryActionID (or \c NONE_e if unknown)

  public:
    class Impl;                 ///< Implementation class
  private:
    RWCOW_pointer<Impl> _pimpl; ///< Pointer to implementation
  protected:
    HistoryLogData & operator=( const HistoryLogData & ); ///< no base class assign
  };

  /** \relates HistoryLogData Stream output */
  std::ostream & operator<<( std::ostream & str, const HistoryLogData & obj );
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class HistoryLogDataInstall
  /// \brief  A zypp history log line for an installed packaged.
  /// \ingroup g_ZyppHistory
  ///////////////////////////////////////////////////////////////////
  class HistoryLogDataInstall : public HistoryLogData
  {
  public:
    typedef shared_ptr<HistoryLogDataInstall>		Ptr;
    typedef shared_ptr<const HistoryLogDataInstall>	constPtr;
    /** Ctor \b moving \a FieldVector (via swap).
     * \throws ParseException if \a fields_r has the wrong \ref HistoryActionID or number of fields.
     */
    HistoryLogDataInstall( FieldVector & fields_r );

  public:
    enum Index			///< indices of known fields
    {
      DATE_INDEX	= HistoryLogData::DATE_INDEX,
      ACTION_INDEX	= HistoryLogData::ACTION_INDEX,
      NAME_INDEX,		///< package name
      EDITION_INDEX,		///< package edition
      ARCH_INDEX,		///< package architecture
      REQBY_INDEX,		///< requested by (user@hostname, pid:appname, or empty (solver))
      REPOALIAS_INDEX,		///< repository providing the package
      CHEKSUM_INDEX,		///< package checksum
      USERDATA_INDEX,		///< userdata/transactionID
    };

   public:
    std::string	name()		const;	///< package name
    Edition	edition()	const;	///< package edition
    Arch	arch()		const;	///< package architecture
    std::string	reqby()		const;	///< requested by (user@hostname, pid:appname, or empty (solver))
    std::string	repoAlias()	const;	///< repository providing the package
    CheckSum	checksum()	const;	///< package checksum
    std::string	userdata()	const;	///< userdata/transactionID
  };

  ///////////////////////////////////////////////////////////////////
  /// \class HistoryLogDataRemove
  /// \brief A zypp history log line for a removed packge.
  /// \ingroup g_ZyppHistory
  ///////////////////////////////////////////////////////////////////
  class HistoryLogDataRemove : public HistoryLogData
  {
  public:
    typedef shared_ptr<HistoryLogDataRemove>		Ptr;
    typedef shared_ptr<const HistoryLogDataRemove>	constPtr;
    /** Ctor \b moving \a FieldVector (via swap).
     * \throws ParseException if \a fields_r has the wrong \ref HistoryActionID or number of fields.
     */
    HistoryLogDataRemove( FieldVector & fields_r );

  public:
    enum Index			///< indices of known fields
    {
      DATE_INDEX	= HistoryLogData::DATE_INDEX,
      ACTION_INDEX	= HistoryLogData::ACTION_INDEX,
      NAME_INDEX,		///< package name
      EDITION_INDEX,		///< package edition
      ARCH_INDEX,		///< package architecture
      REQBY_INDEX,		///< requested by (user@hostname, pid:appname, or empty (solver))
      USERDATA_INDEX,		///< userdata/transactionID
    };

  public:
    std::string	name()		const;	///< package name
    Edition	edition()	const;	///< package edition
    Arch	arch()		const;	///< package architecture
    std::string	reqby()		const;	///< requested by (user@hostname, pid:appname, or empty (solver))
    std::string	userdata()	const;	///< userdata/transactionID
  };

  ///////////////////////////////////////////////////////////////////
  /// \class HistoryLogDataRepoAdd
  /// \brief A zypp history log line for an added repository.
  /// \ingroup g_ZyppHistory
  ///////////////////////////////////////////////////////////////////
  class HistoryLogDataRepoAdd : public HistoryLogData
  {
  public:
    typedef shared_ptr<HistoryLogDataRepoAdd>		Ptr;
    typedef shared_ptr<const HistoryLogDataRepoAdd>	constPtr;
    /** Ctor \b moving \a FieldVector (via swap).
     * \throws ParseException if \a fields_r has the wrong \ref HistoryActionID or number of fields.
     */
    HistoryLogDataRepoAdd( FieldVector & fields_r );

  public:
    enum Index			///< indices of known fields
    {
      DATE_INDEX	= HistoryLogData::DATE_INDEX,
      ACTION_INDEX	= HistoryLogData::ACTION_INDEX,
      ALIAS_INDEX,		///< repository alias
      URL_INDEX,		///< repository url
      USERDATA_INDEX,		///< userdata/transactionID
    };

  public:
    std::string	alias()		const;	///< repository alias
    Url		url()		const;	///< repository url
    std::string	userdata()	const;	///< userdata/transactionID
  };

  ///////////////////////////////////////////////////////////////////
  /// \class HistoryLogDataRepoRemove
  /// \brief A zypp history log line for a removed repository.
  /// \ingroup g_ZyppHistory
  ///////////////////////////////////////////////////////////////////
  class HistoryLogDataRepoRemove : public HistoryLogData
  {
  public:
    typedef shared_ptr<HistoryLogDataRepoRemove>	Ptr;
    typedef shared_ptr<const HistoryLogDataRepoRemove>	constPtr;
    /** Ctor \b moving \a FieldVector (via swap).
     * \throws ParseException if \a fields_r has the wrong \ref HistoryActionID or number of fields.
     */
    HistoryLogDataRepoRemove( FieldVector & fields_r );

  public:
    enum Index			///< indices of known fields
    {
      DATE_INDEX	= HistoryLogData::DATE_INDEX,
      ACTION_INDEX	= HistoryLogData::ACTION_INDEX,
      ALIAS_INDEX,		///< repository alias
      USERDATA_INDEX,		///< userdata/transactionID
    };

  public:
    std::string	alias()		const;	///< repository alias
    std::string	userdata()	const;	///< userdata/transactionID
  };

  ///////////////////////////////////////////////////////////////////
  /// \class HistoryLogDataRepoAliasChange
  /// \brief A zypp history log line for a repo alias change.
  /// \ingroup g_ZyppHistory
  ///////////////////////////////////////////////////////////////////
  class HistoryLogDataRepoAliasChange : public HistoryLogData
  {
  public:
    typedef shared_ptr<HistoryLogDataRepoAliasChange>		Ptr;
    typedef shared_ptr<const HistoryLogDataRepoAliasChange>	constPtr;
    /** Ctor \b moving \a FieldVector (via swap).
     * \throws ParseException if \a fields_r has the wrong \ref HistoryActionID or number of fields.
     */
    HistoryLogDataRepoAliasChange( FieldVector & fields_r );

  public:
    enum Index			///< indices of known fields
    {
      DATE_INDEX	= HistoryLogData::DATE_INDEX,
      ACTION_INDEX	= HistoryLogData::ACTION_INDEX,
      OLDALIAS_INDEX,		///< repositories old alias
      NEWALIAS_INDEX,		///< repositories new alias
      USERDATA_INDEX,		///< userdata/transactionID
   };

  public:
    std::string	oldAlias()	const;	///< repositories old alias
    std::string	newAlias()	const;	///< repositories new alias
    std::string	userdata()	const;	///< userdata/transactionID
  };

  ///////////////////////////////////////////////////////////////////
  /// \class HistoryLogDataRepoUrlChange
  /// \brief A zypp history log line for a repo url change.
  /// \ingroup g_ZyppHistory
  ///////////////////////////////////////////////////////////////////
  class HistoryLogDataRepoUrlChange : public HistoryLogData
  {
  public:
    typedef shared_ptr<HistoryLogDataRepoUrlChange>		Ptr;
    typedef shared_ptr<const HistoryLogDataRepoUrlChange>	constPtr;
    /** Ctor \b moving \a FieldVector (via swap).
     * \throws ParseException if \a fields_r has the wrong \ref HistoryActionID or number of fields.
     */
    HistoryLogDataRepoUrlChange( FieldVector & fields_r );

  public:
    enum Index			///< indices of known fields
    {
      DATE_INDEX	= HistoryLogData::DATE_INDEX,
      ACTION_INDEX	= HistoryLogData::ACTION_INDEX,
      ALIAS_INDEX,		///< repository alias
      NEWURL_INDEX,		///< repositories new url
      USERDATA_INDEX,		///< userdata/transactionID
   };

  public:
    std::string	alias()		const;	///< repository alias
    Url		newUrl()	const;	///< repositories new url
    std::string	userdata()	const;	///< userdata/transactionID
  };

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif /* ZYPP_HISTORYLOGDATA_H_ */
