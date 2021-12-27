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
 * Convenience helpers to automatically generate boilerplate code
 * for pimpl classes.
 *
 * Libzypp is using the PIMPL pattern to ensure binary compatiblity between
 * different version releases. This keeps rebuilds of applications
 * that link against libzypp to a minimum. A PIMPL class simply hides the
 * data members and functions that are not part of the public API/ABI in a
 * hidden private class, that is only accessible in the implementation files.
 * This allows even bigger refactorings to happen behind the scenes.
 *
 * A simple example would be:
 *
 * \code
 *
 * // MyClass.h
 *
 * // forward declare the private class, always use the public classname
 * // with a "Private" postfix:
 * class MyClassPrivate;
 *
 * class MyClass
 * {
 * public:
 *   // add all public API functions here
 *   void doSomething();
 *   int  getSomething() const;
 * private:
 *   // generate the forward declarations for the pimpl access functions
 *   ZYPP_DECLARE_PRIVATE(MyClass)
 *   // the only data member in the public class should be a pointer to the private type
 *   // named d_ptr
 *   std::unique_ptr<MyClassPrivate> d_ptr;
 * };
 *
 * // MyClass.cc
 *
 * // in the implementation file we can now define the private class:
 * class MyClassPrivate
 * {
 * public:
 *   // add the data members and private functions here
 *   int something = 0;
 * };
 *
 * // in the constructor make sure that the private part of the class
 * // is initialized too
 * MyClass::MyClass() : d_ptr( new MyClassPrivate )
 * {}
 *
 * int MyClass::getSomething() const
 * {
 *   // automatically generates a pointer named "d" to the
 *   // pimpl object
 *   Z_D();
 *   return d->something;
 * }
 *
 * void MyClass::doSomething()
 * {
 *   // It is also possible to use the d_func() to access the pointer:
 *   d_func()->something = 10;
 * }
 *
 * \endcode
 *
 * \note those macros are inspired by the Qt framework
 */

template <typename T> inline T *zyppGetPtrHelper(T *ptr) { return ptr; }
template <typename Ptr> inline auto zyppGetPtrHelper(const Ptr &ptr) -> decltype(ptr.operator->()) { return ptr.operator->(); }
template <typename Ptr> inline auto zyppGetPtrHelper(Ptr &ptr) -> decltype(ptr.operator->()) { return ptr.operator->(); }

#define ZYPP_DECLARE_PRIVATE(Class) \
    Class##Private* d_func();\
    const Class##Private* d_func() const; \
    friend class Class##Private;

#define ZYPP_IMPL_PRIVATE(Class) \
    Class##Private* Class::d_func() \
    { return static_cast<Class##Private *>(zyppGetPtrHelper(d_ptr)); } \
    const Class##Private* Class::d_func() const \
    { return static_cast<const Class##Private *>(zyppGetPtrHelper(d_ptr)); }

#define ZYPP_DECLARE_PUBLIC(Class)            \
    public:                                            \
    inline Class* z_func() { return static_cast<Class *>(z_ptr); } \
    inline const Class* z_func() const { return static_cast<const Class *>(z_ptr); } \
    friend class Class; \
    private:

#define Z_D() auto const d = d_func()
#define Z_Z() auto const z = z_func()

#endif
