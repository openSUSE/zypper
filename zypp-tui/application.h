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
* Strictly for internal use!
*/

#ifndef ZYPP_TUI_TUIAPPLICATION_INCLUDED
#define ZYPP_TUI_TUIAPPLICATION_INCLUDED

#include <zypp-core/base/NonCopyable.h>
#include <zypp-tui/Config>
#include <memory>

namespace ztui {

  class Out;
  static constexpr int ZTUI_EXIT_OK = 0;

  class Application : private zypp::base::NonCopyable
  {
  public:

    virtual ~Application();

    /*!
     * Returns the \ref Application instance for the current thread after it
     * was created.
     */
    static Application & instance();

    const Config &config () const;
    Config &mutableConfig();

    virtual Out & out();
    virtual void setOutputWriter( Out * out );

    int exitCode() const { return _exitCode; }
    void setExitCode( int exit );

  protected:
    Application ( std::shared_ptr<Config> &&cfg );

  private:
    Application();
    void init();
    int   _exitCode;
    std::shared_ptr<Config> _config;
    std::shared_ptr<Out> _out;
  };
}



#endif
