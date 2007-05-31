

#ifndef ZYPP_REPOSITORY_IMPL_h
#define ZYPP_REPOSITORY_IMPL_h

#include <iosfwd>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/ProvideNumericId.h"
#include "zypp2/Repository.h"
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(RepositoryImpl);

    class RepositoryImpl : public base::ReferenceCounted,
                          public base::ProvideNumericId<RepositoryImpl,Repository::NumericId>,
                          private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const RepositoryImpl & obj );
    public:
      RepositoryImpl();
      
      ~RepositoryImpl();

      struct null {};
    public:
      /** Offer default Impl. */
      static RepositoryImpl_Ptr nullimpl()
      {
        static RepositoryImpl_Ptr _nullimpl( new RepositoryImpl( null() ) );
        return _nullimpl;
      }

      RepositoryImpl( const null & );

      Repository selfRepository()
      { return Repository( this ); }
    };

  }
}

#endif
