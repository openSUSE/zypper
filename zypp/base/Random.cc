#include <fcntl.h>
#include <unistd.h>
#include "zypp/base/Random.h"

using namespace std;

namespace zypp { namespace base {

 // Taken from KApplication
int random_int()
{
  static bool init = false;
  if (!init)
  {
      unsigned int seed;
      init = true;
      int fd = open("/dev/urandom", O_RDONLY|O_CLOEXEC);
      if (fd < 0 || ::read(fd, &seed, sizeof(seed)) != sizeof(seed))
      {
            // No /dev/urandom... try something else.
            srand(getpid());
            seed = rand()+time(0);
      }
      if (fd >= 0) close(fd);
      srand(seed);
  }
  return rand();
}

// Taken from KApplication
std::string random_string(int length)
{
  if (length <=0 ) return std::string();

  std::string str; str.resize( length );
  int i = 0;
  while (length--)
  {
      int r=::random() % 62;
      r+=48;
      if (r>57) r+=7;
      if (r>90) r+=6;
      str[i++] =  char(r);
      // so what if I work backwards?
  }
  return str;
}


} }

