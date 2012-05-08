/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Target.h
 *
*/
#ifndef ZYPP_TARGET_H
#define ZYPP_TARGET_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/APIConfig.h"

#include "zypp/Product.h"
#include "zypp/Pathname.h"
#include "zypp/ResPool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace target
  {
    class TargetImpl;
    namespace rpm {
      class RpmDb;
    }
  }
  namespace zypp_detail
  {
    class ZYppImpl;
  }

  DEFINE_PTR_TYPE(Target);

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Target
  //
  /**
  */
  class Target : public base::ReferenceCounted, public base::NonCopyable
  {
  public:
    typedef target::TargetImpl  Impl;
    typedef intrusive_ptr<Impl> Impl_Ptr;
    typedef std::list<PoolItem> PoolItemList;

  public:

    /**
     * builds or refreshes the target cache
     */
    void buildCache();

    /**
     * cleans the target cache (.solv files)
     */
    void cleanCache();

   /**
     * load resolvables into the pool
     */
    void load();

    /**
     * unload target resolvables from the
     * pool
     */
    void unload();

    /** Null implementation */
    static Target_Ptr nullimpl();

    /** Refference to the RPM database */
    target::rpm::RpmDb & rpmDb();

    /** If the package is installed and provides the file
     Needed to evaluate split provides during Resolver::Upgrade() */
    bool providesFile (const std::string & name_str, const std::string & path_str) const;

    /** Return name of package owning \a path_str
     * or empty string if no installed package owns \a path_str.
     **/
    std::string whoOwnsFile (const std::string & path_str) const;

    /** Return the root set for this target */
    Pathname root() const;

    /** Return the path prefixed by the target root, unless it already is prefixed. */
    Pathname assertRootPrefix( const Pathname & path_r ) const
    { return Pathname::assertprefix( root(), path_r ); }

    /** return the last modification date of the target */
    Date timestamp() const;

    /**
     * returns the target base installed product, also known as
     * the distribution or platform.
     *
     * returns 0 if there is no base installed product in the
     * pool.
     *
     * \note this method requires the target to be loaded,
     * otherwise it will return 0 as no product is found.
     *
     * if you require some base product attributes when the
     * target is not loaded into the pool, see
     * \ref targetDistribution , \ref targetDistributionRelease
     * and \ref distributionVersion that obtain the data
     * on demand from the installed product information.
     */
    Product::constPtr baseProduct() const;

    /**
     * \brief Languages to be supported by the system.
     * E.g. language specific packages to be installed.
     */
    LocaleSet requestedLocales() const;
    /** \overload Use a specific root_r, if empty the default targets root, or '/'
     */
    static LocaleSet requestedLocales( const Pathname & root_r );

  public:
    /** \name Base product and registration.
     *
     * Static methods herein allow to retrieve the values without explicitly
     * initializing the \ref Target. They take a targets root directory as
     * argument. If an empty \ref Pathname is passed, an already existing
     * Targets root is used, otherwise \c "/" is assumed.
     */
    //@{
    /** This is \c register.target attribute of the installed base product.
     * Used for registration and \ref Service refresh.
     */
    std::string targetDistribution() const;
    /** \overload */
    static std::string targetDistribution( const Pathname & root_r );

    /** This is \c register.release attribute of the installed base product.
     * Used for registration.
     */
    std::string targetDistributionRelease() const;
    /** \overload */
    static std::string targetDistributionRelease( const Pathname & root_r );

    struct DistributionLabel { std::string shortName; std::string summary; };
    /** This is \c shortName and \c summary attribute of the installed base product.
     * Used e.g. for the bootloader menu.
     */
    DistributionLabel distributionLabel() const;
    /** \overload */
    static DistributionLabel distributionLabel( const Pathname & root_r );

    /** This is \c version attribute of the installed base product.
     * For example http://download.opensue.org/update/11.0
     * The 11.0 corresponds to the base product version.
     */
    std::string distributionVersion() const;
    /** \overload */
    static std::string distributionVersion( const Pathname & root_r );

    /**
     * This is \c flavor attribute of the installed base product
     * but does not require the target to be loaded as it remembers
     * the last used one. It can be empty is the target has never
     * been loaded, as the value is not present in the system
     * but computer from a package provides
     */
    std::string distributionFlavor() const;
    /** \overload */
    static std::string distributionFlavor( const Pathname & root_r );

    /**
     * anonymous unique id
     *
     * This id is generated once and stays in the
     * saved in the target.
     * It is unique and is used only for statistics.
     *
     */
    std::string anonymousUniqueId() const;
    /** \overload */
    static std::string anonymousUniqueId( const Pathname & root_r );
    //@}

  public:
    /** Ctor. If \c doRebuild_r is \c true, an already existing
     * database is rebuilt (rpm --rebuilddb ).
    */
    explicit
    Target( const Pathname & root = "/", bool doRebuild_r = false );
    /** Ctor */
    explicit
    Target( const Impl_Ptr & impl_r );

  private:
    friend std::ostream & operator<<( std::ostream & str, const Target & obj );
    /** Stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;

  private:
    /** Direct access to Impl. */
    friend class zypp_detail::ZYppImpl;

    /** Pointer to implementation */
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _pimpl;

    static Target_Ptr _nullimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Target Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Target & obj )
  { return obj.dumpOn( str ); }

  /** \relates Target::DistributionLabel Stream output.
   * Write out the content as key/value pairs:
   * \code
   * summary=Beautiful Name
   * shortName=BN
   * \endcode
   */
  std::ostream & operator<<( std::ostream & str, const Target::DistributionLabel & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_H
