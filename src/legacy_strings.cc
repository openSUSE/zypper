/**
 * \file This file contains old strings that are removed from the main application but need to be kept around
 *       for some time.
 */
void stringsContainer ()
{

#if 0
  static std::string help_global_options = _("  Global Options:\n"
    "\t--help, -h\t\tHelp.\n"
    "\t--version, -V\t\tOutput the version number.\n"
    "\t--promptids\t\tOutput a list of zypper's user prompts.\n"
    "\t--config, -c <file>\tUse specified config file instead of the default.\n"
    "\t--userdata <string>\tUser defined transaction id used in history and plugins.\n"
    "\t--quiet, -q\t\tSuppress normal output, print only error\n"
    "\t\t\t\tmessages.\n"
    "\t--verbose, -v\t\tIncrease verbosity.\n"
    "\t--color\n"
    "\t--no-color\t\tWhether to use colors in output if tty supports it.\n"
    "\t--no-abbrev, -A\t\tDo not abbreviate text in tables.\n"
    "\t--table-style, -s\tTable style (integer).\n"
    "\t--non-interactive, -n\tDo not ask anything, use default answers\n"
    "\t\t\t\tautomatically.\n"
    "\t--non-interactive-include-reboot-patches\n"
    "\t\t\t\tDo not treat patches as interactive, which have\n"
    "\t\t\t\tthe rebootSuggested-flag set.\n"
    "\t--xmlout, -x\t\tSwitch to XML output.\n"
    "\t--ignore-unknown, -i\tIgnore unknown packages.\n"
  );

  static std::string repo_manager_options = _(
    "\t--reposd-dir, -D <dir>\tUse alternative repository definition file\n"
    "\t\t\t\tdirectory.\n"
    "\t--cache-dir, -C <dir>\tUse alternative directory for all caches.\n"
    "\t--raw-cache-dir <dir>\tUse alternative raw meta-data cache directory.\n"
    "\t--solv-cache-dir <dir>\tUse alternative solv file cache directory.\n"
    "\t--pkg-cache-dir <dir>\tUse alternative package cache directory.\n"
  );

  static std::string help_global_repo_options = _("     Repository Options:\n"
    "\t--no-gpg-checks\t\tIgnore GPG check failures and continue.\n"
    "\t--gpg-auto-import-keys\tAutomatically trust and import new repository\n"
    "\t\t\t\tsigning keys.\n"
    "\t--plus-repo, -p <URI>\tUse an additional repository.\n"
    "\t--plus-content <tag>\tAdditionally use disabled repositories providing a specific keyword.\n"
    "\t\t\t\tTry '--plus-content debug' to enable repos indicating to provide debug packages.\n"
    "\t--disable-repositories\tDo not read meta-data from repositories.\n"
    "\t--no-refresh\t\tDo not refresh the repositories.\n"
    "\t--no-cd\t\t\tIgnore CD/DVD repositories.\n"
    "\t--no-remote\t\tIgnore remote repositories.\n"
    "\t--releasever\t\tSet the value of $releasever in all .repo files (default: distribution version)\n"
  );

  static std::string help_global_target_options = _("     Target Options:\n"
    "\t--root, -R <dir>\tOperate on a different root directory.\n"
    "\t--disable-system-resolvables\n"
    "\t\t\t\tDo not read installed packages.\n"
  );
  help_global_target_options += str::Format(
    "\t--installroot <dir>\t%1%\n" ) % _("Operate on a different root directory, but share repositories with the host."
  );

  static std::string help_commands = _(
    "  Commands:\n"
    "\thelp, ?\t\t\tPrint help.\n"
    "\tshell, sh\t\tAccept multiple commands at once.\n"
  );

  static std::string help_repo_commands = _("     Repository Management:\n"
    "\trepos, lr\t\tList all defined repositories.\n"
    "\taddrepo, ar\t\tAdd a new repository.\n"
    "\tremoverepo, rr\t\tRemove specified repository.\n"
    "\trenamerepo, nr\t\tRename specified repository.\n"
    "\tmodifyrepo, mr\t\tModify specified repository.\n"
    "\trefresh, ref\t\tRefresh all repositories.\n"
    "\tclean\t\t\tClean local caches.\n"
  );

  static std::string help_service_commands = _("     Service Management:\n"
    "\tservices, ls\t\tList all defined services.\n"
    "\taddservice, as\t\tAdd a new service.\n"
    "\tmodifyservice, ms\tModify specified service.\n"
    "\tremoveservice, rs\tRemove specified service.\n"
    "\trefresh-services, refs\tRefresh all services.\n"
  );

  static std::string help_package_commands = _("     Software Management:\n"
    "\tinstall, in\t\tInstall packages.\n"
    "\tremove, rm\t\tRemove packages.\n"
    "\tverify, ve\t\tVerify integrity of package dependencies.\n"
    "\tsource-install, si\tInstall source packages and their build\n"
    "\t\t\t\tdependencies.\n"
    "\tinstall-new-recommends, inr\n"
    "\t\t\t\tInstall newly added packages recommended\n"
    "\t\t\t\tby installed packages.\n"
  );

  static std::string help_update_commands = _("     Update Management:\n"
    "\tupdate, up\t\tUpdate installed packages with newer versions.\n"
    "\tlist-updates, lu\tList available updates.\n"
    "\tpatch\t\t\tInstall needed patches.\n"
    "\tlist-patches, lp\tList needed patches.\n"
    "\tdist-upgrade, dup\tPerform a distribution upgrade.\n"
    "\tpatch-check, pchk\tCheck for patches.\n"
  );

  static std::string help_query_commands = _("     Querying:\n"
    "\tsearch, se\t\tSearch for packages matching a pattern.\n"
    "\tinfo, if\t\tShow full information for specified packages.\n"
    "\tpatch-info\t\tShow full information for specified patches.\n"
    "\tpattern-info\t\tShow full information for specified patterns.\n"
    "\tproduct-info\t\tShow full information for specified products.\n"
    "\tpatches, pch\t\tList all available patches.\n"
    "\tpackages, pa\t\tList all available packages.\n"
    "\tpatterns, pt\t\tList all available patterns.\n"
    "\tproducts, pd\t\tList all available products.\n"
    "\twhat-provides, wp\tList packages providing specified capability.\n"
    //"\twhat-requires, wr\tList packages requiring specified capability.\n"
    //"\twhat-conflicts, wc\tList packages conflicting with specified capability.\n"
  );

  static std::string help_lock_commands = _("     Package Locks:\n"
    "\taddlock, al\t\tAdd a package lock.\n"
    "\tremovelock, rl\t\tRemove a package lock.\n"
    "\tlocks, ll\t\tList current package locks.\n"
    "\tcleanlocks, cl\t\tRemove unused locks.\n"
  );

  static std::string help_other_commands = _("     Other Commands:\n"
    "\tversioncmp, vcmp\tCompare two version strings.\n"
    "\ttargetos, tos\t\tPrint the target operating system ID string.\n"
    "\tlicenses\t\tPrint report about licenses and EULAs of\n"
    "\t\t\t\tinstalled packages.\n"
    "\tdownload\t\tDownload rpms specified on the commandline to a local directory.\n"
    "\tsource-download\t\tDownload source rpms for all installed packages\n"
    "\t\t\t\tto a local directory.\n"
  );

  static std::string help_subcommands = _("     Subcommands:\n"
    "\tsubcommand\t\tLists available subcommands.\n"
  );

  static std::string help_usage = _(
    "  Usage:\n"
    "\tzypper [--global-options] <command> [--command-options] [arguments]\n"
    "\tzypper <subcommand> [--command-options] [arguments]\n"
  );

  zypper.out().info( help_usage, Out::QUIET );
  zypper.out().info( help_global_options, Out::QUIET );
  zypper.out().info( repo_manager_options, Out::QUIET );
  zypper.out().info( help_global_repo_options, Out::QUIET );
  zypper.out().info( help_global_target_options, Out::QUIET );
  zypper.out().info( help_commands, Out::QUIET );
  zypper.out().info( help_repo_commands, Out::QUIET );
  zypper.out().info( help_service_commands, Out::QUIET );
  zypper.out().info( help_package_commands, Out::QUIET );
  zypper.out().info( help_update_commands, Out::QUIET );
  zypper.out().info( help_query_commands, Out::QUIET );
  zypper.out().info( help_lock_commands, Out::QUIET );
  zypper.out().info( help_other_commands, Out::QUIET );
  zypper.out().info( help_subcommands, Out::QUIET );

  print_command_help_hint( zypper );

    _command_help = ( CommandHelpFormater()
      << str::form(_(
      // translators: the first %s = "package, patch, pattern, product",
      // second %s = "package",
      // and the third %s = "only, in-advance, in-heaps, as-needed"
      "install (in) [OPTIONS] <capability|rpm_file_uri> ...\n"
      "\n"
      "Install packages with specified capabilities or RPM files with specified\n"
      "location. A capability is NAME[.ARCH][OP<VERSION>], where OP is one\n"
      "of <, <=, =, >=, >.\n"
      "\n"
      "  Command options:\n"
      "    --from <alias|#|URI>    Select packages from the specified repository.\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-t, --type <type>           Type of package (%s).\n"
      "                            Default: %s.\n"
      "-n, --name                  Select packages by plain name, not by capability.\n"
      "-C, --capability            Select packages by capability.\n"
      "-f, --force                 Install even if the item is already installed (reinstall),\n"
      "                            downgraded or changes vendor or architecture.\n"
      "    --oldpackage            Allow to replace a newer item with an older one.\n"
      "                            Handy if you are doing a rollback. Unlike --force\n"
      "                            it will not enforce a reinstall.\n"
      "    --replacefiles          Install the packages even if they replace files from other,\n"
      "                            already installed, packages. Default is to treat file conflicts\n"
      "                            as an error. --download-as-needed disables the fileconflict check.\n"
      "-l, --auto-agree-with-licenses\n"
      "                            Automatically say 'yes' to third party license\n"
      "                            confirmation prompt.\n"
      "                            See 'man zypper' for more details.\n"
      "-D, --dry-run               Test the installation, do not actually install.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
    ), "package, patch, pattern, product, srcpackage",
       "package",
       "only, in-advance, in-heaps, as-needed") )


    _command_help = ( CommandHelpFormater()
      << str::form(_(
      // TranslatorExplanation the first %s = "package, patch, pattern, product"
      //  and the second %s = "package"
      "remove (rm) [OPTIONS] <capability> ...\n"
      "\n"
      "Remove packages with specified capabilities.\n"
      "A capability is NAME[.ARCH][OP<VERSION>], where OP is one\n"
      "of <, <=, =, >=, >.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-t, --type <type>           Type of package (%s).\n"
      "                            Default: %s.\n"
      "-n, --name                  Select packages by plain name, not by capability.\n"
      "-C, --capability            Select packages by capability.\n"
      "-u, --clean-deps            Automatically remove unneeded dependencies.\n"
      "-U, --no-clean-deps         No automatic removal of unneeded dependencies.\n"
      "-D, --dry-run               Test the removal, do not actually remove.\n"
      "    --details               Show the detailed installation summary.\n"
      ), "package, patch, pattern, product", "package") )

    _command_help = ( CommandHelpFormater()
    << _(
      "source-install (si) [OPTIONS] <name> ...\n"
      "\n"
      "Install specified source packages and their build dependencies.\n"
      "\n"
      "  Command options:\n"
      "-d, --build-deps-only    Install only build dependencies of specified packages.\n"
      "-D, --no-build-deps      Don't install build dependencies.\n"
      "-r, --repo <alias|#|URI> Install packages only from specified repositories.\n"
      "    --download-only      Only download the packages, do not install.\n"
    ) )

    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "verify (ve) [OPTIONS]\n"
      "\n"
      "Check whether dependencies of installed packages are satisfied"
      " and suggest to install or remove packages in order to repair the"
      " dependency problems.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-D, --dry-run               Test the repair, do not actually do anything to\n"
      "                            the system.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
      ), "only, in-advance, in-heaps, as-needed") )

    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "install-new-recommends (inr) [OPTIONS]\n"
      "\n"
      "Install newly added packages recommended by already installed packages."
      " This can typically be used to install new language packages or drivers"
      " for newly added hardware.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>    Load only the specified repositories.\n"
      "-D, --dry-run               Test the installation, do not actually install.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
    ), "only, in-advance, in-heaps, as-needed") )

    _(
      // translators: the %s = "ris" (the only service type currently supported)
      "addservice (as) [OPTIONS] <URI> <alias>\n"
      "\n"
      "Add a repository index service to the system.\n"
      "\n"
      "  Command options:\n"
      "-t, --type <type>       Type of the service (%s).\n"
      "-d, --disable           Add the service as disabled.\n"
      "-n, --name <name>       Specify descriptive name for the service.\n"
    )

    _command_help = _(
      // TranslatorExplanation the %s = "yast2, rpm-md, plaindir"
      "removeservice (rs) [OPTIONS] <alias|#|URI>\n"
      "\n"
      "Remove specified repository index service from the system..\n"
      "\n"
      "  Command options:\n"
      "    --loose-auth   Ignore user authentication data in the URI.\n"
      "    --loose-query  Ignore query string in the URI.\n"
    );

    _(
      // translators: %s is "--all" and "--all"
      "modifyservice (ms) <options> <alias|#|URI>\n"
      "modifyservice (ms) <options> <%s>\n"
      "\n"
      "Modify properties of services specified by alias, number, or URI, or by the\n"
      "'%s' aggregate options.\n"
      "\n"
      "  Command options:\n"
      "-d, --disable                  Disable the service (but don't remove it).\n"
      "-e, --enable                   Enable a disabled service.\n"
      "-r, --refresh                  Enable auto-refresh of the service.\n"
      "-R, --no-refresh               Disable auto-refresh of the service.\n"
      "-n, --name <name>              Set a descriptive name for the service.\n"
      "\n"
      "-i, --ar-to-enable <alias>     Add a RIS service repository to enable.\n"
      "-I, --ar-to-disable <alias>    Add a RIS service repository to disable.\n"
      "-j, --rr-to-enable <alias>     Remove a RIS service repository to enable.\n"
      "-J, --rr-to-disable <alias>    Remove a RIS service repository to disable.\n"
      "-k, --cl-to-enable             Clear the list of RIS repositories to enable.\n"
      "-K, --cl-to-disable            Clear the list of RIS repositories to disable.\n"
      "\n"
      "-a, --all                      Apply changes to all services.\n"
      "-l, --local                    Apply changes to all local services.\n"
      "-t, --remote                   Apply changes to all remote services.\n"
      "-m, --medium-type <type>       Apply changes to services of specified type.\n"
    )

    ZypperCommand::LIST_SERVICES_e:
    _command_help = _(
      "services (ls) [OPTIONS]\n"
      "\n"
      "List defined services.\n"
      "\n"
      "  Command options:\n"
      "-u, --uri                 Show also base URI of repositories.\n"
      "-p, --priority            Show also repository priority.\n"
      "-d, --details             Show more information like URI, priority, type.\n"
      "-r, --with-repos          Show also repositories belonging to the services.\n"
      "-E, --show-enabled-only   Show enabled repos only.\n"
      "-P, --sort-by-priority    Sort the list by repository priority.\n"
      "-U, --sort-by-uri         Sort the list by URI.\n"
      "-N, --sort-by-name        Sort the list by name.\n"
    );

    //ZypperCommand::REFRESH_SERVICES_e
    _command_help = _(
      "refresh-services (refs) [OPTIONS]\n"
      "\n"
      "Refresh defined repository index services.\n"
      "\n"
      "  Command options:\n"
      "-f, --force           Force a complete refresh.\n"
      "-r, --with-repos      Refresh also the service repositories.\n"
      "-R, --restore-status  Also restore service repositories enabled/disabled state.\n"
    );


    _(
      // translators: the %s = "yast2, rpm-md, plaindir"
      "addrepo (ar) [OPTIONS] <URI> <alias>\n"
      "addrepo (ar) [OPTIONS] <file.repo>\n"
      "\n"
      "Add a repository to the system. The repository can be specified by its URI"
      " or can be read from specified .repo file (even remote).\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <file.repo>    Just another means to specify a .repo file to read.\n"
      "-t, --type <type>         Type of repository (%s).\n"
      "-d, --disable             Add the repository as disabled.\n"
      "-c, --check               Probe URI.\n"
      "-C, --no-check            Don't probe URI, probe later during refresh.\n"
      "-n, --name <name>         Specify descriptive name for the repository.\n"
      "-p, --priority <integer>  Set priority of the repository.\n"
      "-k, --keep-packages       Enable RPM files caching.\n"
      "-K, --no-keep-packages    Disable RPM files caching.\n"
      "-f, --refresh             Enable autorefresh of the repository.\n"
    )

    _command_help = _(
      "repos (lr) [OPTIONS] [repo] ...\n"
      "\n"
      "List all defined repositories.\n"
      "\n"
      "  Command options:\n"
      "-e, --export <FILE.repo>  Export all defined repositories as a single local .repo file.\n"
      "-a, --alias               Show also repository alias.\n"
      "-n, --name                Show also repository name.\n"
      "-u, --uri                 Show also base URI of repositories.\n"
      "-p, --priority            Show also repository priority.\n"
      "-r, --refresh             Show also the autorefresh flag.\n"
      "-d, --details             Show more information like URI, priority, type.\n"
      "-s, --service             Show also alias of parent service.\n"
      "-E, --show-enabled-only   Show enabled repos only.\n"
      "-U, --sort-by-uri         Sort the list by URI.\n"
      "-P, --sort-by-priority    Sort the list by repository priority.\n"
      "-A, --sort-by-alias       Sort the list by alias.\n"
      "-N, --sort-by-name        Sort the list by name.\n"
    );

  _command_help = ( CommandHelpFormater() << _(
    "removerepo (rr) [OPTIONS] <alias|#|URI>\n"
    "\n"
    "Remove repository specified by alias, number or URI.\n"
    "\n"
    "  Command options:\n"
    "    --loose-auth   Ignore user authentication data in the URI.\n"
    "    --loose-query  Ignore query string in the URI.\n"
  ))

    _command_help = _(
      "renamerepo (nr) [OPTIONS] <alias|#|URI> <new-alias>\n"
      "\n"
      "Assign new alias to the repository specified by alias, number or URI.\n"
      "\n"
      "This command has no additional options.\n"
    );

    _(
      // translators: %s is "--all|--remote|--local|--medium-type"
      // and "--all, --remote, --local, --medium-type"
      "modifyrepo (mr) <options> <alias|#|URI> ...\n"
      "modifyrepo (mr) <options> <%s>\n"
      "\n"
      "Modify properties of repositories specified by alias, number, or URI, or by the\n"
      "'%s' aggregate options.\n"
      "\n"
      "  Command options:\n"
      "-d, --disable             Disable the repository (but don't remove it).\n"
      "-e, --enable              Enable a disabled repository.\n"
      "-r, --refresh             Enable auto-refresh of the repository.\n"
      "-R, --no-refresh          Disable auto-refresh of the repository.\n"
      "-n, --name <name>         Set a descriptive name for the repository.\n"
      "-p, --priority <integer>  Set priority of the repository.\n"
      "-k, --keep-packages       Enable RPM files caching.\n"
      "-K, --no-keep-packages    Disable RPM files caching.\n"
    )
    _(
      "-a, --all                 Apply changes to all repositories.\n"
      "-l, --local               Apply changes to all local repositories.\n"
      "-t, --remote              Apply changes to all remote repositories.\n"
      "-m, --medium-type <type>  Apply changes to repositories of specified type.\n"
    )

    _command_help = _(
      "refresh (ref) [alias|#|URI] ...\n"
      "\n"
      "Refresh repositories specified by their alias, number or URI."
      " If none are specified, all enabled repositories will be refreshed.\n"
      "\n"
      "  Command options:\n"
      "-f, --force              Force a complete refresh.\n"
      "-b, --force-build        Force rebuild of the database.\n"
      "-d, --force-download     Force download of raw metadata.\n"
      "-B, --build-only         Only build the database, don't download metadata.\n"
      "-D, --download-only      Only download raw metadata, don't build the database.\n"
      "-r, --repo <alias|#|URI> Refresh only specified repositories.\n"
      "-s, --services           Refresh also services before refreshing repos.\n"
    );

    _command_help = _(
      "clean (cc) [alias|#|URI] ...\n"
      "\n"
      "Clean local caches.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI> Clean only specified repositories.\n"
      "-m, --metadata           Clean metadata cache.\n"
      "-M, --raw-metadata       Clean raw metadata cache.\n"
      "-a, --all                Clean both metadata and package caches.\n"
    );

    _command_help = ( CommandHelpFormater()
      << str::form(_(
      // translators: the first %s = "package, patch, pattern, product",
      // the second %s = "patch",
      // and the third %s = "only, in-avance, in-heaps, as-needed"
      "update (up) [OPTIONS] [packagename] ...\n"
      "\n"
      "Update all or specified installed packages with newer versions, if possible.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-t, --type <type>           Type of package (%s).\n"
      "                            Default: %s.\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "    --skip-interactive      Skip interactive updates.\n"
      "    --with-interactive      Do not skip interactive updates.\n"
      "-l, --auto-agree-with-licenses\n"
      "                            Automatically say 'yes' to third party license\n"
      "                            confirmation prompt.\n"
      "                            See man zypper for more details.\n"
      "    --best-effort           Do a 'best effort' approach to update. Updates\n"
      "                            to a lower than the latest version are\n"
      "                            also acceptable.\n"
      "    --replacefiles          Install the packages even if they replace files from other,\n"
      "                            already installed, packages. Default is to treat file conflicts\n"
      "                            as an error. --download-as-needed disables the fileconflict check.\n"
      "-D, --dry-run               Test the update, do not actually update.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
      ), "package, patch, pattern, product, srcpackage",
         "package",
         "only, in-advance, in-heaps, as-needed") )

    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "patch [OPTIONS]\n"
      "\n"
      "Install all available needed patches.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "    --skip-interactive      Skip interactive patches.\n"
      "    --with-interactive      Do not skip interactive patches.\n"
      "-l, --auto-agree-with-licenses\n"
      "                            Automatically say 'yes' to third party license\n"
      "                            confirmation prompt.\n"
      "                            See man zypper for more details.\n"
      "-b, --bugzilla #            Install patch fixing the specified bugzilla issue.\n"
      "    --cve #                 Install patch fixing the specified CVE issue.\n"
      "-g  --category <category>   Install only patches with this category.\n"
      "    --severity <severity>   Install only patches with this severity.\n"
      "    --date <YYYY-MM-DD>     Install only patches issued up to, but not including, the specified date\n"
      "    --replacefiles          Install the packages even if they replace files from other,\n"
      "                            already installed, packages. Default is to treat file conflicts\n"
      "                            as an error. --download-as-needed disables the fileconflict check.\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-D, --dry-run               Test the update, do not actually update.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
      ), "only, in-advance, in-heaps, as-needed") )

        _command_help = ( CommandHelpFormater()
          << str::form(_(
          "dist-upgrade (dup) [OPTIONS]\n"
          "\n"
          "Perform a distribution upgrade.\n"
          "\n"
          "  Command options:\n"
          "\n"
          "    --from <alias|#|URI>    Restrict upgrade to specified repository.\n"
          "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
          "-l, --auto-agree-with-licenses\n"
          "                            Automatically say 'yes' to third party license\n"
          "                            confirmation prompt.\n"
          "                            See man zypper for more details.\n"
          "    --replacefiles          Install the packages even if they replace files from other,\n"
          "                            already installed, packages. Default is to treat file conflicts\n"
          "                            as an error. --download-as-needed disables the fileconflict check.\n"
          "-D, --dry-run               Test the upgrade, do not actually upgrade\n"
          "    --details               Show the detailed installation summary.\n"
          "    --download              Set the download-install mode. Available modes:\n"
          "                            %s\n"
          "-d, --download-only         Only download the packages, do not install.\n"
          ), "only, in-advance, in-heaps, as-needed") )


        _command_help = str::form(_(
          // TranslatorExplanation the first %s = "package, patch, pattern, product"
          //  and the second %s = "patch"
          "list-updates (lu) [OPTIONS]\n"
          "\n"
          "List all available updates.\n"
          "\n"
          "  Command options:\n"
          "-t, --type <type>             Type of package (%s).\n"
          "                              Default: %s.\n"
          "-r, --repo <alias|#|URI>      List only updates from the specified repository.\n"
          "    --best-effort             Do a 'best effort' approach to update. Updates\n"
          "                              to a lower than the latest version are\n"
          "                              also acceptable.\n"
          "-a, --all                     List all packages for which newer versions are\n"
          "                              available, regardless whether they are\n"
          "                              installable or not.\n"
        ), "package, patch, pattern, product", "package");

    case ZypperCommand::LIST_PATCHES_e:
    .option(_("-b, --bugzilla[=#]"		"\n"	"List applicable patches for Bugzilla issues."))
    .option(_(    "--cve[=#]"			"\n"	"List applicable patches for CVE issues."))
    .option(_(    "--issues[=STRING]"		"\n"	"Look for issues matching the specified string."))
    .option(_(    "--date <YYYY-MM-DD>"		"\n"	"List only patches issued up to, but not including, the specified date."))
    .option(_("-g, --category <CATEGORY>"	"\n"	"List only patches with this category."))
    .option(_(    "--severity <SEVERITY>"	"\n"	"List only patches with this severity."))
    .option(_("-a, --all"			"\n"	"List all patches, not only applicable ones."))
    .option_WITHout_OPTIONAL
    .option(_("-r, --repo <ALIAS|#|URI>"	"\n"	"List only patches from the specified repository."))

        _command_help = _(
          "patches (pch) [repository] ...\n"
          "\n"
          "List all patches available in specified repositories.\n"
          "\n"
          "  Command options:\n"
          "\n"
          "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
        );

        _command_help = _(
          "packages (pa) [OPTIONS] [repository] ...\n"
          "\n"
          "List all packages available in specified repositories.\n"
          "\n"
          "  Command options:\n"
          "\n"
          "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
          "-i, --installed-only      Show only installed packages.\n"
          "-u, --not-installed-only  Show only packages which are not installed.\n"
          "    --orphaned            Show packages which are orphaned (without repository).\n"
          "    --suggested           Show packages which are suggested.\n"
          "    --recommended         Show packages which are recommended.\n"
          "    --unneeded            Show packages which are unneeded.\n"
          "-N, --sort-by-name        Sort the list by package name.\n"
          "-R, --sort-by-repo        Sort the list by repository.\n"
        );

        _command_help = _(
          "patterns (pt) [OPTIONS] [repository] ...\n"
          "\n"
          "List all patterns available in specified repositories.\n"
          "\n"
          "  Command options:\n"
          "\n"
          "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
          "-i, --installed-only      Show only installed patterns.\n"
          "-u, --not-installed-only  Show only patterns which are not installed.\n"
        );

        _command_help = ( CommandHelpFormater()
          << _(
          "products (pd) [OPTIONS] [repository] ...\n"
          "\n"
          "List all products available in specified repositories.\n"
          "\n"
          "  Command options:\n"
          "\n"
          "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
          "-i, --installed-only      Show only installed products.\n"
          "-u, --not-installed-only  Show only products which are not installed.\n") )

        _command_help = ( CommandHelpFormater() << str::form(_(
            "info (if) [OPTIONS] <name> ...\n"
            "\n"
            "Show detailed information for specified packages.\n"
            "By default the packages which match exactly the given names are shown.\n"
            "To get also packages partially matching use option '--match-substrings'\n"
            "or use wildcards (*?) in name.\n"
            "\n"
            "  Command options:\n"
            "-s, --match-substrings    Print information for packages partially matching name.\n"
            "-r, --repo <alias|#|URI>  Work only with the specified repository.\n"
            "-t, --type <type>         Type of package (%s).\n"
            "                          Default: %s.\n"
            "    --provides            Show provides.\n"
            "    --requires            Show requires and prerequires.\n"
            "    --conflicts           Show conflicts.\n"
            "    --obsoletes           Show obsoletes.\n"
            "    --recommends          Show recommends.\n"
            "    --suggests            Show suggests.\n"
          ), "package, patch, pattern, product", "package"))

        _command_help = str::form(_(
          "patch-info <patchname> ...\n"
          "\n"
          "Show detailed information for patches.\n"
          "\n"
          "This is an alias for '%s'.\n"
        ), "zypper info -t patch");

        _command_help = str::form(_(
          "pattern-info <pattern_name> ...\n"
          "\n"
          "Show detailed information for patterns.\n"
          "\n"
          "This is an alias for '%s'.\n"
        ), "zypper info -t pattern");

        _command_help = str::form(_(
          "product-info <product_name> ...\n"
          "\n"
          "Show detailed information for products.\n"
          "\n"
          "This is an alias for '%s'.\n"
        ), "zypper info -t product");

        _command_help = _(
          "what-provides (wp) <capability>\n"
          "\n"
          "List all packages providing the specified capability.\n"
          "\n"
          "This command has no additional options.\n"
        );

        _command_help = _(
          "moo\n"
          "\n"
          "Show an animal.\n"
          "\n"
          "This command has no additional options.\n"
          );

    _command_help = _(
      "targetos (tos) [OPTIONS]\n"
      "\n"
      "Show various information about the target operating system.\n"
      "By default, an ID string is shown.\n"
      "\n"
      "  Command options:\n"
      "-l, --label                 Show the operating system label.\n"
    );


    _command_help = _(
      "versioncmp (vcmp) <version1> <version2>\n"
      "\n"
      "Compare the versions supplied as arguments.\n"
      "\n"
      "  Command options:\n"
      "-m, --match  Takes missing release number as any release.\n"
    );

    _command_help = _(
      "licenses\n"
      "\n"
      "Report licenses and EULAs of currently installed software packages.\n"
      "\n"
      "This command has no additional options.\n"
    );

    _command_help = _(
      "download [OPTIONS] <PACKAGES>...\n"
      "\n"
      "Download rpms specified on the commandline to a local directory.\n"
      "Per default packages are downloaded to the libzypp package cache\n"
      "(/var/cache/zypp/packages; for non-root users $XDG_CACHE_HOME/zypp/packages),\n"
      "but this can be changed by using the global --pkg-cache-dir option.\n"
      "\n"
      "In XML output a <download-result> node is written for each\n"
      "package zypper tried to download. Upon success the local path is\n"
      "is found in 'download-result/localpath@path'.\n"
      "\n"
      "  Command options:\n"
      "--all-matches        Download all versions matching the commandline\n"
      "                     arguments. Otherwise only the best version of\n"
      "                     each matching package is downloaded.\n"
      "--dry-run            Don't download any package, just report what\n"
      "                     would be done.\n"
    );

    _command_help = _(
      "source-download\n"
      "\n"
      "Download source rpms for all installed packages to a local directory.\n"
      "\n"
      "  Command options:\n"
      "-d, --directory <dir>\n"
      "                     Download all source rpms to this directory.\n"
      "                     Default: /var/cache/zypper/source-download\n"
      "--delete             Delete extraneous source rpms in the local directory.\n"
      "--no-delete          Do not delete extraneous source rpms.\n"
      "--status             Don't download any source rpms,\n"
      "                     but show which source rpms are missing or extraneous.\n"
    );
//       "--manifest           Write MANIFEST of packages and coresponding source rpms.\n"
//       "--no-manifest        Do not write MANIFEST.\n"

    _command_help = _(
      "shell (sh)\n"
      "\n"
      "Enter the zypper command shell.\n"
      "\n"
      "This command has no additional options.\n"
    );

    _command_help = _(
      // translators: this is just a legacy command
      "ping [OPTIONS]\n"
      "\n"
      "This command has dummy implementation which always returns 0.\n"
    );

    _command_help = ( CommandHelpFormater() << _(
      "search (se) [OPTIONS] [querystring] ...\n"
      "\n"
      "Search for packages matching any of the given search strings.\n"
      "\n"
      "  Command options:\n"
      "    --match-substrings     Search for a match to partial words (default).\n"
      "    --match-words          Search for a match to whole words only.\n"
      "-x, --match-exact          Searches for an exact match of the search strings.\n"
      "    --provides             Search for packages which provide the search strings.\n"
      "    --recommends           Search for packages which recommend the search strings.\n"
      "    --requires             Search for packages which require the search strings.\n"
      "    --suggests             Search for packages which suggest the search strings.\n"
      "    --conflicts            Search packages conflicting with search strings.\n"
      "    --obsoletes            Search for packages which obsolete the search strings.\n"
      "-n, --name                 Useful together with dependency options, otherwise\n"
      "                           searching in package name is default.\n"
      "-f, --file-list            Search for a match in the file list of packages.\n"
      "-d, --search-descriptions  Search also in package summaries and descriptions.\n"
      "-C, --case-sensitive       Perform case-sensitive search.\n"
      "-i, --installed-only       Show only installed packages.\n"
      "-u, --not-installed-only   Show only packages which are not installed.\n"
      "-t, --type <type>          Search only for packages of the specified type.\n"
      "-r, --repo <alias|#|URI>   Search only in the specified repository.\n"
      "    --sort-by-name         Sort packages by name (default).\n"
      "    --sort-by-repo         Sort packages by repository.\n"
      "-s, --details              Show each available version in each repository\n"
      "                           on a separate line.\n"
      "-v, --verbose              Like --details, with additional information where the\n"
      "                           search has matched (useful for search in dependencies).\n"
      "\n"
      "* and ? wildcards can also be used within search strings.\n"
      "If a search string is enclosed in '/', it's interpreted as a regular expression.\n"
    ))

    _command_help = _(
      "quit (exit, ^D)\n"
      "\n"
      "Quit the current zypper shell.\n"
      "\n"
      "This command has no additional options.\n"
    );
    #endif
}
