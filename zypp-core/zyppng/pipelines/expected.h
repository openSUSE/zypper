/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
* Based on code by Ivan Čukić (BSD/MIT licensed) from the functional cpp book
*/

#ifndef ZYPP_ZYPPNG_MONADIC_EXPECTED_H
#define ZYPP_ZYPPNG_MONADIC_EXPECTED_H

#include <zypp-core/zyppng/meta/Functional>
#include <zypp-core/zyppng/pipelines/AsyncResult>

namespace zyppng {

  template<typename T, typename E = std::exception_ptr>
  class expected {
  protected:
      union {
          T m_value;
          E m_error;
      };

      bool m_isValid;

      expected() // used internally
      {
      }

  public:

    using value_type = T;
    using error_type = E;

      ~expected()
      {
          if (m_isValid) {
              m_value.~T();
          } else {
              m_error.~E();
          }
      }

      expected(const expected &other)
          : m_isValid(other.m_isValid)
      {
          if (m_isValid) {
              new (&m_value) T(other.m_value);
          } else {
              new (&m_error) E(other.m_error);
          }
      }

      expected(expected &&other)
          : m_isValid(other.m_isValid)
      {
          if (m_isValid) {
              new (&m_value) T( std::move(other.m_value) );
          } else {
              new (&m_error) E( std::move(other.m_error) );
          }
      }

      expected &operator= (expected other)
      {
          swap(other);
          return *this;
      }

      void swap(expected &other)
      {
          using std::swap;
          if (m_isValid) {
              if (other.m_isValid) {
                  // Both are valid, just swap the values
                  swap(m_value, other.m_value);

              } else {
                  // We are valid, but the other one is not
                  // we need to do the whole dance
                  auto temp = std::move(other.m_error);       // moving the error into the temp
                  other.m_error.~E();                         // destroying the original error object
                  new (&other.m_value) T(std::move(m_value)); // moving our value into the other
                  m_value.~T();                               // destroying our value object
                  new (&m_error) E(std::move(temp));          // moving the error saved to the temp into us
                  std::swap(m_isValid, other.m_isValid);      // swap the isValid flags
              }

          } else {
              if (other.m_isValid) {
                  // We are not valid, but the other one is,
                  // just call swap on other and rely on the
                  // implementation in the previous case
                  other.swap(*this);

              } else {
                  // Everything is rotten, just swap the errors
                  swap(m_error, other.m_error);
                  std::swap(m_isValid, other.m_isValid);
              }
          }
      }

      template <typename... ConsParams>
      static expected success(ConsParams && ...params)
      {
          expected result;
          result.m_isValid = true;
          new(&result.m_value) T(std::forward<ConsParams>(params)...);
          return result;
      }

      template <typename... ConsParams>
      static expected error(ConsParams && ...params)
      {
          expected result;
          result.m_isValid = false;
          new(&result.m_error) E(std::forward<ConsParams>(params)...);
          return result;
      }

      operator bool() const
      {
          return m_isValid;
      }

      bool is_valid() const
      {
          return m_isValid;
      }

      #ifdef NO_EXCEPTIONS
      #    define THROW_IF_EXCEPTIONS_ARE_ENABLED(WHAT) std::terminate()
      #else
      #    define THROW_IF_EXCEPTIONS_ARE_ENABLED(WHAT) throw std::logic_error(WHAT)
      #endif

      T &get()
      {
          if (!m_isValid) THROW_IF_EXCEPTIONS_ARE_ENABLED("expected<T, E> contains no value");
          return m_value;
      }

      const T &get() const
      {
          if (!m_isValid) THROW_IF_EXCEPTIONS_ARE_ENABLED("expected<T, E> contains no value");
          return m_value;
      }



      T *operator-> ()
      {
          return &get();
      }

      const T *operator-> () const
      {
          return &get();
      }

      E &error()
      {
          if (m_isValid) THROW_IF_EXCEPTIONS_ARE_ENABLED("There is no error in this expected<T, E>");
          return m_error;
      }

      const E &error() const
      {
          if (m_isValid) THROW_IF_EXCEPTIONS_ARE_ENABLED("There is no error in this expected<T, E>");
          return m_error;
      }

      #undef THROW_IF_EXCEPTIONS_ARE_ENABLED

      template <typename F>
      void visit(F f) {
          if (m_isValid) {
              f(m_value);
          } else {
              f(m_error);
          }
      }
  };


  template<typename E>
  class expected<void, E> {
  private:
      union {
          void* m_value;
          E m_error;
      };

      bool m_isValid;

      expected() {} //used internally

  public:
      ~expected()
      {
          if (m_isValid) {
              // m_value.~T();
          } else {
              m_error.~E();
          }
      }

      expected(const expected &other)
          : m_isValid(other.m_isValid)
      {
          if (m_isValid) {
              // new (&m_value) T(other.m_value);
          } else {
              new (&m_error) E(other.m_error);
          }
      }

      expected(expected &&other)
          : m_isValid(other.m_isValid)
      {
          if (m_isValid) {
              // new (&m_value) T(std::move(other.m_value));
          } else {
              new (&m_error) E(std::move(other.m_error));
          }
      }

      expected &operator= (expected other)
      {
          swap(other);
          return *this;
      }

      void swap(expected &other)
      {
          using std::swap;
          if (m_isValid) {
              if (other.m_isValid) {
                  // Both are valid, we do not have any values
                  // to swap

              } else {
                  // We are valid, but the other one is not.
                  // We need to move the error into us
                  auto temp = std::move(other.m_error);    // moving the error into the temp
                  other.m_error.~E();                      // destroying the original error object
                  new (&m_error) E(std::move(temp));       // moving the error into us
                  std::swap(m_isValid, other.m_isValid);   // swapping the isValid flags
              }

          } else {
              if (other.m_isValid) {
                  // We are not valid, but the other one is,
                  // just call swap on other and rely on the
                  // implementation in the previous case
                  other.swap(*this);

              } else {
                  // Everything is rotten, just swap the errors
                  swap(m_error, other.m_error);
                  std::swap(m_isValid, other.m_isValid);
              }
          }
      }

      static expected success()
      {
          expected result;
          result.m_isValid = true;
          result.m_value = nullptr;
          return result;
      }

      template <typename... ConsParams>
      static expected error(ConsParams && ...params)
      {
          expected result;
          result.m_isValid = false;
          new(&result.m_error) E(std::forward<ConsParams>(params)...);
          return result;
      }

      operator bool() const
      {
          return m_isValid;
      }

      bool is_valid() const
      {
          return m_isValid;
      };

      #ifdef NO_EXCEPTIONS
      #    define THROW_IF_EXCEPTIONS_ARE_ENABLED(WHAT) std::terminate()
      #else
      #    define THROW_IF_EXCEPTIONS_ARE_ENABLED(WHAT) throw std::logic_error(WHAT)
      #endif

      E &error()
      {
          if (m_isValid) THROW_IF_EXCEPTIONS_ARE_ENABLED("There is no error in this expected<T, E>");
          return m_error;
      }

      const E &error() const
      {
          if (m_isValid) THROW_IF_EXCEPTIONS_ARE_ENABLED("There is no error in this expected<T, E>");
          return m_error;
      }

  };


  template < typename T
           , typename E
           , typename Function
           , typename ResultType = decltype(std::declval<Function>()(std::declval<T>()))
           >
  std::enable_if_t< !detail::is_async_op< remove_smart_ptr_t<ResultType> >::value, ResultType> mbind( const expected<T, E>& exp, Function f)
  {
      if (exp) {
        return std::invoke( f, exp.get() );
      } else {
          return ResultType::error(exp.error());
      }
  }

  template < typename T
    , typename E
    , typename Function
    , typename ResultType = decltype(std::declval<Function>()(std::declval<T>()))
    >
  std::enable_if_t< !detail::is_async_op< remove_smart_ptr_t<ResultType> >::value, ResultType> mbind( expected<T, E>&& exp, Function f)
  {
    if (exp) {
      return std::invoke( f, std::move(exp.get()) );
    } else {
      return ResultType::error( std::move(exp.error()) );
    }
  }

  template < typename T
    , typename E
    , typename Function
    , typename ResultType = decltype(std::declval<Function>()(std::declval<T>()))
    >
  std::enable_if_t< detail::is_async_op< remove_smart_ptr_t<ResultType> >::value, ResultType> mbind( const expected<T, E>& exp, Function f)
  {
    if (exp) {
      return std::invoke( f, exp.get() );
    } else {
      return  makeReadyResult( remove_smart_ptr_t<ResultType>::value_type::error(exp.error()) );
    }
  }

  template < typename T
    , typename E
    , typename Function
    , typename ResultType = decltype(std::declval<Function>()(std::declval<T>()))
    >
  std::enable_if_t< detail::is_async_op< remove_smart_ptr_t<ResultType> >::value, ResultType> mbind( expected<T, E>&& exp, Function f)
  {
    if (exp) {
      return std::invoke( f, std::move(exp.get()) );
    } else {
      return  makeReadyResult( remove_smart_ptr_t<ResultType>::value_type::error( std::move(exp.error()) ) );
    }
  }

  namespace detail {
    template <typename Callback>
    struct mbind_helper {
      Callback function;

      template< typename T
        , typename E >
      auto operator()( const expected<T, E>& exp ) {
        return mbind( exp, function );
      }

      template< typename T
        , typename E >
      auto operator()( expected<T, E>&& exp ) {
        return mbind( std::move(exp), function );
      }
    };
  }

  namespace operators {
    template <typename Fun>
    auto mbind ( Fun && function ) {
      return detail::mbind_helper<Fun> {
        std::forward<Fun>(function)
      };
    }
  }
}

#endif

