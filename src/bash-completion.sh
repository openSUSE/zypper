# zypper completion v 0.2 aplha 2 :-)
# A hackweek gift from Marek Stopka <mstopka@opensuse.org>
#
# some TODOs:
# - complete package names for install/remove/update

_zypper() {
        ZYPPER_CMDLIST=()
	ZYPPER="/usr/bin/zypper -q"
	local magic_string="Command options:"
	local opts cur prev prevprev
	if test ${#ZYPPER_CMDLIST[*]} = 0; then
		for foo in $(LC_ALL=C $ZYPPER -h | sed -e "1,/Commands:/d" | awk -F ' ' '{print $1} {print $2}' | sed -e 's/,//' -e 's/[[:upper:]].*//'); do
			ZYPPER_CMDLIST="$ZYPPER_CMDLIST $foo"
		done
	fi

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}
	if [[ ${#COMP_WORDS[@]} -ge 3 ]]; then
		prevprev=${COMP_WORDS[COMP_CWORD-2]}
	fi

	case "$prev" in
		zypper)
			opts=$ZYPPER_CMDLIST
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		help | ?)
		
			opts=$ZYPPER_CMDLIST
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		shell | sh)
			return 0
		;;
		repos | lr)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		addrepo | ar)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		removerepo | rr)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      opts="${opts} $(LC_ALL=C $ZYPPER  lr | sed -e '1,2 d' -e 's/^[0-9]\+[[:space:]]\+|[[:space:]]*\([^|]\+\)|.*$/\1/' -e 's/[[:space:]]*$//')"
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		renamerepo | nr)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      opts="${opts} $(LC_ALL=C $ZYPPER  lr | sed -e '1,2 d' -e 's/^[0-9]\+[[:space:]]\+|[[:space:]]*\([^|]\+\)|.*$/\1/' -e 's/[[:space:]]*$//')"
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		modifyrepo | mr)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      opts2=$(LC_ALL=C $ZYPPER  lr | sed -e '1,2 d' -e 's/^[0-9]\+[[:space:]]\+|[[:space:]]*\([^|]\+\)|.*$/\1/' -e 's/[[:space:]]*$//'  -e 's/ /\\ /g' -e "s/^\(.*\)$/'\1'/")
      opts="${opts} ${opts2}"
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		refresh | ref)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		clean)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		services | ls)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		addservice | as)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      opts2=$(LC_ALL=C $ZYPPER  ls | sed -e '1,2 d' -e 's/^[0-9]\+[[:space:]]\+|[[:space:]]*\([^|]\+\)|.*$/\1/' -e 's/[[:space:]]*$//'  -e 's/ /\\ /g' -e "s/^\(.*\)$/'\1'/")
      opts="${opts} ${opts2}"
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		modifyservice | ms)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      opts2=$(LC_ALL=C $ZYPPER  ls | sed -e '1,2 d' -e 's/^[0-9]\+[[:space:]]\+|[[:space:]]*\([^|]\+\)|.*$/\1/' -e 's/[[:space:]]*$//'  -e 's/ /\\ /g' -e "s/^\(.*\)$/'\1'/")
      opts="${opts} ${opts2}"
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		removeservice | rs)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      opts2=$(LC_ALL=C $ZYPPER  ls | sed -e '1,2 d' -e 's/^[0-9]\+[[:space:]]\+|[[:space:]]*\([^|]\+\)|.*$/\1/' -e 's/[[:space:]]*$//'  -e 's/ /\\ /g' -e "s/^\(.*\)$/'\1'/")
      opts="${opts} ${opts2}"
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		refresh-services | refs)
			opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		install | in)
			opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		remove | rm)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		verify | ve)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//' | grep -v '*')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		source-install | si)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d"  -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		install-new-recommends | inr)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d"  -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		update | up)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d"  -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		list-updates | lup)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		patch)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		list-updates)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		dist-upgrade | dup)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d"  -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		patch-check | pchk)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d"  -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		search | se)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//' | grep -v '*')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		info | if)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//' | grep -v '*')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		patch-info)
      COMPREPLY=""
      return 0
		;;
		pattern-info)
      COMPREPLY=""
      return 0
		;;
		product-info)
      COMPREPLY=""
      return 0
		;;
		patches | pch)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		packages | pa)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		patterns | pt)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		products | pd)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		what-provides | wp)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		addlock | al)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		removelock | rl)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		locks | ll)
      opts=$(LC_ALL=C $ZYPPER help $prev 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
      COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
		"--type")
						opts="package patch pattern product"
						COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		;;
	esac
}

complete -F _zypper -o default zypper
