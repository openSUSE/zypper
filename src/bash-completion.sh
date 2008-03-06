# zypper completion v 0.2 aplha 1 :-) (This is not feature complete version!)
# A hackweek gift from Marek Stopka <marekstopka@gmail.com>
#
# some TODOs:
# - complete package names for install/remove/update
# - make sure the default file completion always works for:
#   * install (to install plain rpms)
#   * addrepo --repo (to specify .repo file)
#   * maybe other cases   

_zypper() {
	ZYPPER="/usr/bin/zypper"
	local magic_string="Command options:"
	local opts cur prev prevprev
	if test ${#MODLIST[*]} = 0; then
		for foo in $(LC_ALL=C $ZYPPER -h | sed -e "1,/Commands:/d" | awk -F ' ' '{print $1}' | sed 's/,//'); do
			MODLIST="$MODLIST $foo"
		done
	fi

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}
	if [[ ${#COMP_WORDS[@]} -ge 3 ]]; then
		prevprev=${COMP_WORDS[COMP_CWORD-2]}
	fi

	case "$prev" in
		zypper)
      opts=$MODLIST
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		help)
			opts=$MODLIST
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		shell)
			return 0
		;;
		install)
			opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		remove)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		search)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//' | grep -v '*')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		repos)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		addrepo)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		removerepo)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		renamerepo)
      return 0
		;;
		modifyrepo)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		refresh)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		patch_check)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		patches)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		list-updates)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		xml-updates)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		update)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d"  -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		info)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d"  -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		patch-info)
      return 0
		;;
		source-install)
      return 0
		;;
		"--type")
						opts="package patch pattern product"
						COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		dist-upgrade)
			opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d"  -e 's/.*--/--/' -e 's/ .*//')
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
	esac
}

complete -F _zypper zypper
