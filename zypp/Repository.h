
#ifndef ZYPP_REPOSITORY_H
#define ZYPP_REPOSITORY_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/SafeBool.h"
//#include "zypp/ResStore.h"
#include "zypp/RepoInfo.h"
#include "zypp/repo/PackageDelta.h"

namespace zypp
{
  class ResStore;

  namespace repo
  {
    DEFINE_PTR_TYPE(RepositoryImpl);
    class RepositoryImpl;
  }

  class Repository : private base::SafeBool<Repository>
  {
  public:
    typedef repo::RepositoryImpl     Impl;
    typedef repo::RepositoryImpl_Ptr Impl_Ptr;

  public:

    /**
     * \short Default ctor: noRepository.
     * \see RepoManager::createFromCache.
    */
    Repository();

    /** \short Factory ctor taking a RepositoryImpl.
     * This is quite lowevel. You usually want to use RepoManager to
     * manage the repositories.
     * \see RepoManager
    */
    explicit
    Repository( const Impl_Ptr & impl_r );

    /**
     * A dummy Repository (Id \c 0) providing nothing, doing nothing.
    */
    static const Repository noRepository;

    /** Validate Repository in a boolean context.
     * \c FALSE iff == noRepository.
    */
    using base::SafeBool<Repository>::operator bool_type;

  public:
    typedef unsigned long NumericId;

    /** Runtime unique numeric Repository Id. */
    NumericId numericId() const;

    /**
     * \short Get the resolvables for repo
     */
    const zypp::ResStore & resolvables() const;

    /**
     * \short Repository info used to create this repository
     */
    const RepoInfo & info() const;

    /**
     * \short Patch RPMs the repository provides
     */
    const std::list<packagedelta::PatchRpm> & patchRpms() const;

    /**
     * \short Delta RPMs the repository provides
     */
    const std::list<packagedelta::DeltaRpm> & deltaRpms() const;

  private:
    friend base::SafeBool<Repository>::operator bool_type() const;
    /** \ref SafeBool test. */
    bool boolTest() const
    { return _pimpl != noRepository._pimpl; }

  private:
    /** Pointer to implementation */
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _pimpl;
  };

  /** \relates Repository */
  std::ostream & operator<<( std::ostream & str, const Repository & obj );
  /** \relates Repository */
  bool operator==( const Repository & lhs, const Repository & rhs );
  /** \relates Repository */
  inline bool operator!=( const Repository & lhs, const Repository & rhs )
  { return !( lhs == rhs ); }
  /** \relates Repository */
  bool operator<( const Repository & lhs, const Repository & rhs );

}

#endif


