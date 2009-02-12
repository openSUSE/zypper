# zypper completion v 0.2 aplha 2 :-)
# A hackweek gift from Marek Stopka <mstopka@opensuse.org>
#
# some TODOs:
# - complete package names for install/remove/update

_zypper() {
        ZYPPER_CMDLIST=()
	ZYPPER="/usr/bin/zypper -q"
	local magic_string="Command options:"
	local opts opts2 cur prev command ITER
	if test ${#ZYPPER_CMDLIST[*]} = 0; then
		for foo in $(LC_ALL=C $ZYPPER -h | sed -e "1,/Commands:/d" | awk -F ' ' '{print $1} {print $2}' | sed -e 's/,//' -e 's/[[:upper:]].*//'); do
			ZYPPER_CMDLIST="${ZYPPER_CMDLIST} $foo"
		done
		ZYPPER_CMDLIST="${ZYPPER_CMDLIST} "
	fi

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}
	eval "ITER=$COMP_CWORD-1"
	while [[ ${ITER} -ge 0 ]]; do
		opts2=${COMP_WORDS[ITER]}
		if [[ "${ZYPPER_CMDLIST}" =~ " ${opts2} " ]]; then
			command=${COMP_WORDS[ITER]}
			break;
		fi
		if [[ "${opts2}" =~ "zypper" ]]; then

			command="zypper"
			break;
		fi
		eval "ITER=$ITER-1"
	done

	case "$prev" in
		"--type" | "-t")
			opts="package patch pattern product"
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
			return 0;
		;;
		"--repo" | "-r")
			opts="${opts} $(LC_ALL=C $ZYPPER  lr | sed -e '1,2 d' -e 's/^[0-9]\+[[:space:]]\+|[[:space:]]*\([^|]\+\)|.*$/\1/' -e 's/[[:space:]]*$//')"
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
			return 0;
		;;
	esac
	
	if [[ "$command" =~ "zypper" ]]; then
		opts=$ZYPPER_CMDLIST
      		opts2=$(LC_ALL=C $ZYPPER help 2>&1 | sed -e "1,/Global Options:/d" -e "/Commands:/,$ d" -e 's/.*--/--/' -e 's/[,[:space:]].*//' -e '/^$/d')
		opts="${opts} ${opts2}"
		COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
		return 0;
	fi

	if [ -n "$command" ]; then
      		opts=$(LC_ALL=C $ZYPPER help $command 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')
		
		#handling individual commands if they need more then we can dig from help
		case "$command" in
			help | ?)
				opts=$ZYPPER_CMDLIST
			;;
			removerepo | rr | modifyrepo | mr | renamerepo | nr | refresh | ref)
      				opts2=$(LC_ALL=C $ZYPPER  lr | sed -e '1,2 d' -e 's/^[0-9]\+[[:space:]]\+|[[:space:]]*\([^|]\+\)|.*$/\1/' -e 's/[[:space:]]*$//'  -e 's/ /\\ /g' -e "s/^\(.*\)$/'\1'/")
				opts="${opts} ${opts2}"
			;;
			addservice | as | modifyservice | ms | removeservice | rs)
      				opts2=$(LC_ALL=C $ZYPPER  ls | sed -e '1,2 d' -e 's/^[0-9]\+[[:space:]]\+|[[:space:]]*\([^|]\+\)|.*$/\1/' -e 's/[[:space:]]*$//'  -e 's/ /\\ /g' -e "s/^\(.*\)$/'\1'/")
				opts="${opts} ${opts2}"
			;;
			removelock | rl)
				opts2=$(LC_ALL=C $ZYPPER  ll | sed -e '1,2 d' | cut -d '|' -f 2)
				opts="${opts} ${opts2}"
			;;
		esac
			
		COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
	fi
}

complete -F _zypper -o default zypper
