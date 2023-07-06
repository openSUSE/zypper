/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#include "application.h"
#include <zypp-tui/output/OutNormal.h>
#include <zypp-core/base/Exception.h>

namespace ztui {

  namespace {
    Application **getApplicationInstance () {
      thread_local Application *app = nullptr;
      return &app;
    }
  }

  Application::Application()
    : _exitCode(ZTUI_EXIT_OK)
    , _config( new Config() )
  { init(); }

  Application::Application(std::shared_ptr<Config> &&cfg)
    : _exitCode(ZTUI_EXIT_OK)
    , _config( std::move(cfg) )
  { init(); }

  void Application::init()
  {
    // ALWAYS do this FIRST!
    *getApplicationInstance() = this;
    _out = std::make_shared<OutNormal>();
  }

  Application::~Application()
  {
    // set the thread local global to null again
    *getApplicationInstance() = nullptr;
  }

  Application &Application::instance()
  {
    auto instPtr = *getApplicationInstance ();
    if ( !instPtr )
      ZYPP_THROW( zypp::Exception("No ztui::Application intance registered, its required to create one manually before using ztui.") );
    return *instPtr;
  }

  const Config &Application::config() const
  {
    return *_config;
  }

  Config &Application::mutableConfig()
  {
    return *_config;
  }

  Out &Application::out()
  {
    if ( not _out ) {
      _out.reset(new OutNormal( Out::QUIET ));
    }
    return *_out;
  }

  void Application::setOutputWriter(Out *out)
  {
    if ( out == _out.get() )
      return;
    _out.reset ( out );
  }

  void Application::setExitCode(int exit)			{
    WAR << "setExitCode " << exit << std::endl;
    _exitCode = exit;
  }

}
