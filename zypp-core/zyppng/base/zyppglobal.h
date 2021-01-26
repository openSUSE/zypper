#ifndef ZYPP_NG_BASE_ZYPPGLOBAL_H_INCLUDED
#define ZYPP_NG_BASE_ZYPPGLOBAL_H_INCLUDED

#include <zypp-core/base/Easy.h>

#ifndef EXPORT_EXPERIMENTAL_API
#define LIBZYPP_NG_EXPORT
#define LIBZYPP_NG_NO_EXPORT
#else
#include <zypp-ng_export.h>
#endif

/*
 * \note those macros are inspired by the Qt framework
 */

template <typename T> inline T *zyppGetPtrHelper(T *ptr) { return ptr; }
template <typename Ptr> inline auto zyppGetPtrHelper(const Ptr &ptr) -> decltype(ptr.operator->()) { return ptr.operator->(); }
template <typename Ptr> inline auto zyppGetPtrHelper(Ptr &ptr) -> decltype(ptr.operator->()) { return ptr.operator->(); }

#define ZYPP_DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() \
    { return reinterpret_cast<Class##Private *>(zyppGetPtrHelper(d_ptr)); } \
    inline const Class##Private* d_func() const \
    { return reinterpret_cast<const Class##Private *>(zyppGetPtrHelper(d_ptr)); } \
    friend class Class##Private;

#define ZYPP_FWD_DECLARE_PRIVATE(Class) \
    Class##Private* d_func();\
    const Class##Private* d_func() const; \
    friend class Class##Private;

#define ZYPP_IMPL_PRIVATE(Class) \
    Class##Private* Class::d_func() \
    { return reinterpret_cast<Class##Private *>(zyppGetPtrHelper(d_ptr)); } \
    const Class##Private* Class::d_func() const \
    { return reinterpret_cast<const Class##Private *>(zyppGetPtrHelper(d_ptr)); }

#define ZYPP_DECLARE_PUBLIC(Class)            \
    public:                                            \
    inline Class* z_func() { return static_cast<Class *>(z_ptr); } \
    inline const Class* z_func() const { return static_cast<const Class *>(z_ptr); } \
    friend class Class; \
    private:

#define Z_D() auto const d = d_func()
#define Z_Z() auto const z = z_func()

#endif
