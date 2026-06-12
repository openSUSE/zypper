/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "conditions.h"
#include "global-settings.h"
#include "Zypper.h"
#include "transactionalwrapper.h"

int NeedsRootCondition::check(std::string &err)
{
  if ( geteuid() != 0 && !Zypper::instance().config().changedRoot )
  {
    err = _("Root privileges are required to run this command.");
    return ZYPPER_EXIT_ERR_PRIVILEGES;
  }
  return ZYPPER_EXIT_OK;
}

int NeedsWritableRoot::check(std::string &err_r)
{
  Zypper &zypper = Zypper::instance();
  const Config &gopts = zypper.config();

  if ( DryRunSettings::instance().isEnabled() )
    return ZYPPER_EXIT_OK;

  static const std::string msgTS           {_("Transactional system detected:")};
  static const std::string msgNoWrapper    {_("A transactional-wrapper command is not installed.")};
  static const std::string msgNoWrapperSH  {_("The transactional-wrapper command cannot be used in 'zypper shell'.")};
  static const std::string msgNoWrapperEnv {_("Using transactional-wrapper is disabled by 'ZYPPER_USE_TRANSACTIONAL_WRAPPER=0'.")};
  static const std::string msgUseTU        {_("Please use transactional-update to modify or update the system.")};

  if ( readOnlyRootAt( gopts.root_dir ) ) {
    if ( gopts.root_dir == "/" && TransactionalWrapper::isTransactionalSystem() ) {
      if ( TransactionalWrapper::hasTransactionalWrapper() ) {
        if ( TransactionalWrapper::mayUseTransactionalWrapper() ) {
          if ( zypper.runningShell() ) {
            err_r = str::sprintln( msgTS, msgNoWrapperSH );
            return ZYPPER_EXIT_ERR_PRIVILEGES;
          } else {
            zypper.out().info( MSG_WARNINGString( str::sprintln( msgTS, _("Delegating the command to transactional-wrapper.") ) ).str() );
            //Exception will be handled and will run transactional-wrapper
            ZYPP_THROW( TransactionalWrapperException() );
          }
        } else {
          err_r = str::sprintln( msgTS, msgNoWrapperEnv, msgUseTU );
          return ZYPPER_EXIT_ERR_PRIVILEGES;
        }
      } else {
        err_r = str::sprintln( msgTS, msgNoWrapper, msgUseTU );
        return ZYPPER_EXIT_ERR_PRIVILEGES;
      }
    } else {
      err_r = _("The target filesystem is mounted as read-only. Please make sure the target filesystem is writeable.");
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }
  }

  return ZYPPER_EXIT_OK;
}
