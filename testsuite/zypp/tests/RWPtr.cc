#include <zypp/base/PtrTypes.h>
#include <string>
#include <iostream>

struct Foo
{
  int _foo;

  Foo(int foo=0): _foo(foo)
  {
    std::cerr << "created Foo(" << _foo << ")" << std::endl;
  }
  ~Foo()
  {
    std::cerr << "destroy Foo(" << _foo << ")" << std::endl;
  }
};

#define REF_TEST(ref,msg,exp,res) \
do { \
  bool unique = exp; \
  std::cerr << msg << std::endl; \
  if( ref) { \
    std::cerr << "ref contains object" << std::endl; \
  } else { \
    std::cerr << "ref contains no object" << std::endl; \
  } \
  std::cerr << "ref counter is " << ref.use_count() << std::endl; \
  if( ref.unique()) { \
    std::cerr << "ref is unique" << std::endl; \
    if( unique) { \
      std::cerr << "EXPECTED" << std::endl; \
    } else { \
      std::cerr << "NOT EXPECTED" << std::endl; \
      res = 1; \
    } \
  } else { \
    std::cerr << "ref is shared" << std::endl; \
    if( !unique) { \
      std::cerr << "EXPECTED" << std::endl; \
    } else { \
      std::cerr << "NOT EXPECTED" << std::endl; \
      res = 1;  \
    } \
  } \
  std::cerr << std::endl; \
} while(0);

int main(int argc, char *argv[])
{
  (void)argv;

  bool skip_reset = argc > 1;
  int  result = 0;

  typedef zypp::RW_pointer<Foo> FooRef;

  FooRef ref;
  REF_TEST(ref,"=== REF(nil)", true, result);

  ref.reset(new Foo(42));
  REF_TEST(ref,"=== REF(object)", true, result);

  {
    FooRef ref2(ref);
    REF_TEST(ref,"=== REF2(REF)", false, result);
  }

  REF_TEST(ref,"=== REF(object), REF2 out of scope now", true, result);

  if( !skip_reset)
  {
    ref.reset();
    REF_TEST(ref,"=== REF(nil), reset()", true, result);
  }

  std::cerr << "RESULT: "
            << (result == 0 ? "PASSED" : "FAILED")
            << std::endl;
  return result;
}

