#ifndef ZYPPNG_IO_PRIVATE_FORKSPAWNENGINE_H
#define ZYPPNG_IO_PRIVATE_FORKSPAWNENGINE_H

#include "abstractspawnengine_p.h"

namespace zyppng {

  class AbstractDirectSpawnEngine : public AbstractSpawnEngine
  {
  public:
    ~AbstractDirectSpawnEngine();
    virtual bool isRunning ( bool wait = false ) override;
  };

  /*!
    \internal
    Process forking engine thats using the traditional fork() approach
   */
  class ForkSpawnEngine : public AbstractDirectSpawnEngine
  {
  public:
    bool start( const char *const *argv, int stdin_fd, int stdout_fd, int stderr_fd  ) override;
    bool usePty () const;
    void setUsePty ( const bool set = true );

  private:
    /**
     * Set to true, if a pair of ttys is used for communication
     * instead of a pair of pipes.
     */
    bool _use_pty = false;
  };

  /*!
    \internal
    Process forking engine thats using g_spawn from glib which can in most cases optimize
    using posix_spawn.
   */
  class GlibSpawnEngine : public AbstractDirectSpawnEngine
  {
  public:
    bool start( const char *const *argv, int stdin_fd, int stdout_fd, int stderr_fd  ) override;

  private:
    static void glibSpawnCallback ( void *data );
  };

}


#endif //
