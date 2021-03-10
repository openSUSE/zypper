/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/VendorAttr.h
*/
#ifndef ZYPP_VENDORATTR_H
#define ZYPP_VENDORATTR_H

#include <iosfwd>
#include <string>
#include <vector>

#include <zypp/base/PtrTypes.h>
#include <zypp/IdString.h>
#include <zypp/PathInfo.h>
#include <zypp/Vendor.h>

#include <zypp/APIConfig.h>	// LEGACY macros

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

  class PoolItem;
  namespace sat
  {
    class Solvable;
  }

/** Definition of vendor equivalence.
 *
 * Packages with equivalent vendor strings may replace themselves without
 * creating a solver error.
 *
 * Per default vendor strings starting with \c "suse" are treated as
 * being equivalent. This may be tuned by providing customized
 * vendor description files in \c /etc/zypp/vendors.d.
 *
 * \code
 * [main]
 * vendors = MyVendor,AlternateName
 * \endcode
 *
 * By this vendor strings starting with "MyVendor" or "AlternateName"
 * are considered to be equivalent. Packages from equivalent vendors
 * may replace each other without being considered as a 'vendor change'.
 *
 * \note bnc#812608: Within the "opensuse*" namespace exact matches
 * (case insensitive) are required. "vendors = suse,opensuse" will
 * allow switching between "suse*" and "opensuse", but not e.g.
 * "opensuse build service".
 *
 * \see \ref pg_zypp-solv-vendorchange
*/
class VendorAttr
{
    friend std::ostream & operator<<( std::ostream & str, const VendorAttr & obj );

  public:
    /** (Pseudo)Singleton, mapped to the current \ref Target::vendorAttr settings or to \ref noTargetInstance. */
    static const VendorAttr & instance();

    /** Singleton, settings used if no \ref Target is active.
     * The instance is initialized with the settings found in the system below /.
     * The active Targets settings can be changed via \ref Target::vendorAttr.
     */
    static VendorAttr & noTargetInstance();

  public:
    /** Ctor providing the default set. */
    VendorAttr();

    /** Ctor reading the \a initial_r definitions from a dir or file. */
    VendorAttr( const Pathname & initial_r );

    /** Dtor */
    ~VendorAttr();

    /**
     * Adding new equivalent vendors described in a directory
     **/
    bool addVendorDirectory( const Pathname & dirname_r );
#if LEGACY(1722)
    /** \deprecated This is NOT a CONST operation. */
    bool addVendorDirectory( const Pathname & dirname_r ) const ZYPP_DEPRECATED;
#endif

    /**
     * Adding new equivalent vendors described in a file
     **/
    bool addVendorFile( const Pathname & filename_r );
#if LEGACY(1722)
    /** \deprecated This is NOT a CONST operation. */
    bool addVendorFile( const Pathname & filename_r ) const ZYPP_DEPRECATED;
#endif

    /** Preferred type to pass equivalent vendor strings. */
    typedef std::vector<std::string> VendorList;

    /**
     * Adding new equivalent vendor strings container.
     **/
    void addVendorList( VendorList && list_r )
    { _addVendorList( std::move(list_r) ); }
    /** \overload copying the data */
    void addVendorList( const VendorList & list_r )
    { _addVendorList( VendorList(list_r) ); }
    /** \overload from arbitrary string container */
    template <class TContainer>
    void addVendorList( const TContainer & container_r )
    {
      VendorList tmp;
      for ( const auto & el : container_r )
	tmp.push_back( std::string(el) );
      _addVendorList( std::move(tmp) );
    }
    /** \overload from std::initializer_list */
    template <class TStr>
    void addVendorList( const std::initializer_list<TStr> & container_r )
    {
      VendorList tmp;
      for ( const auto & el : container_r )
	tmp.push_back( std::string(el) );
      _addVendorList( std::move(tmp) );
    }

    /** Return whether two vendor strings should be treated as the same vendor.
     * Usually the solver is allowed to automatically select a package of an
     * equivalent vendor when updating. Replacing a package with one of a
     * different vendor usually must be confirmed by the user.
    */
    bool equivalent( const Vendor & lVendor, const Vendor & rVendor ) const;
    /** \overload using \ref IdStrings */
    bool equivalent( IdString lVendor, IdString rVendor ) const;
    /** \overload using \ref sat::Solvable */
    bool equivalent( sat::Solvable lVendor, sat::Solvable rVendor ) const;
    /** \overload using \ref PoolItem */
    bool equivalent( const PoolItem & lVendor, const PoolItem & rVendor ) const;

    /** Like \ref equivalent but always unifies suse and openSUSE vendor */
    bool relaxedEquivalent( const Vendor & lVendor, const Vendor & rVendor ) const;
    /** \overload using \ref IdStrings */
    bool relaxedEquivalent( IdString lVendor, IdString rVendor ) const;
    /** \overload using \ref sat::Solvable */
    bool relaxedEquivalent( sat::Solvable lVendor, sat::Solvable rVendor ) const;
    /** \overload using \ref PoolItem */
    bool relaxedEquivalent( const PoolItem & lVendor, const PoolItem & rVendor ) const;

  public:
    /** Call \a fnc_r for each equivalent vendor list (return \c false to break).
     * \return The number of calls to \a fnc_r.
     */
    unsigned foreachVendorList( std::function<bool(VendorList)> fnc_r ) const;

  public:
    class Impl;                 ///< Implementation class.
    RWCOW_pointer<Impl> _pimpl; ///< Pointer to implementation.

#if LEGACY(1722)
    /** \deprecated */
    void _addVendorList( std::vector<std::string> & list_r ) const ZYPP_DEPRECATED;
    /** \deprecated */
    void _addVendorList( std::vector<IdString> && list_r );
#endif
    void _addVendorList( VendorList && list_r );
};

/** \relates VendorAttr Stream output */
std::ostream & operator<<( std::ostream & str, const VendorAttr & obj );

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_VENDORATTR_H
