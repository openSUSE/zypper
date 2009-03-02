# zypper bash completion script
#
# A hackweek gift from Marek Stopka <mstopka@opensuse.org>
# Major rewrite by Josef Reidinger <jreidinger@suse.cz>
# 2009/02/19 Allow empty spaces in repos names, Werner Fink <werner@suse.de>
#
# some TODOs:
# - complete package names for install/remove/update

_strip()
{
	local s c o
	if test ${#COMPREPLY[@]} -gt 0 ; then
		s="${COMP_WORDBREAKS// }"
		s="${s//	}"
		s="${s//[\{\}()\[\]]}"
		s="${s} 	(){}[]"
		o=${#s}
		while test $((o--)) -gt 0 ; do
			c="${s:${o}:1}"
			COMPREPLY=(${COMPREPLY[*]//${c}/\\${c}})
		done
	fi
}

_zypper() {
        ZYPPER_CMDLIST=()
	ZYPPER="$(type -p zypper)"

	local noglob=$(shopt -po noglob)
	local magic_string="Command options:"
	local comp cur prev command
	local -a opts=()
	local -i ITER=0
	local IFS=$'\n'

	# Do not expand `?' for help
	set -o noglob

	if test ${#ZYPPER_CMDLIST[@]} -eq 0; then
		ZYPPER_CMDLIST=($(LC_ALL=POSIX $ZYPPER -q -h | \
				sed -rn '/^[[:blank:]]*Commands:/,$ {
					/[\t]{4}/d
					s/^[[:blank:]]*//
					s/^[[:upper:]].*://
					s/[[:blank:]]+[[:upper:]].*$//
					s/,[[:blank:]]+/\n/
					s/\?/\\?/
					/^$/d
					p
				}'))
	fi

	if test $COMP_CWORD -lt 1 ; then
		let COMP_CWORD=${#COMP_WORDS[@]}
	fi
	prev=${COMP_WORDS[COMP_CWORD-1]}
	cur=${COMP_WORDS[COMP_CWORD]}

	let ITER=COMP_CWORD
	while test $((ITER--)) -ge 0 ; do
		comp="${COMP_WORDS[ITER]}"
		if [[ "${ZYPPER_CMDLIST[@]}" =~ "${comp}" ]]; then
			command=${COMP_WORDS[ITER]}
			break;
		fi
		if [[ "${comp}" =~ "zypper" ]]; then
			command="zypper"
			break;
		fi
	done
	unset ITER comp

	case "$prev" in
		"--type" | "-t")
			opts=(package patch pattern product)
			COMPREPLY=($(compgen -W "${opts[*]}" -- ${cur}))
			_strip
			eval $noglob
			return 0;
		;;
		"--repo" | "-r")
			opts=(${opts[@]}$(echo; LC_ALL=POSIX $ZYPPER -q lr | \
				sed -rn '/^[0-9]/{
					s/^[0-9]+[[:blank:]]*\|[[:blank:]]*([^|]+).*/\1/
					s/[[:blank:]]*$//
					p
				}'))
			COMPREPLY=($(compgen -W "${opts[*]}" -- ${cur}))
			_strip
			eval $noglob
			return 0;
		;;
	esac
	unset prev
	
	if [[ "$command" =~ "zypper" ]]; then
		opts=(${ZYPPER_CMDLIST[*]}$(echo; LC_ALL=POSIX $ZYPPER -q help 2>&1 | \
			sed -rn '/Global Options:/,/Commands:/{
				/[\t]{4}/d
				s/^[[:blank:]]*//
				/[[:upper:]].*:/d
				s/[[:blank:]]+[[:upper:]].*$//
				s/[,[:blank:]].*$/\n/
				/^$/d
				p
			}'))
		COMPREPLY=($(compgen -W "${opts[*]}" -- ${cur}))
		_strip
		eval $noglob
		return 0;
	fi

	if test -n "$command" ; then
      		opts=$(LC_ALL=POSIX $ZYPPER -q help $command 2>&1 | sed -e "1,/$magic_string/d" -e 's/.*--/--/' -e 's/ .*//')

		#handling individual commands if they need more then we can dig from help
		case "$command" in
			help | \?)
				opts=(${ZYPPER_CMDLIST[@]})
			;;
			removerepo | rr | modifyrepo | mr | renamerepo | nr | refresh | ref)
				opts=(${opts[@]}$(echo; LC_ALL=POSIX $ZYPPER -q lr | \
					sed -rn '/^[0-9]/{
						s/^[0-9]+[[:blank:]]*\|[[:blank:]]*([^|]+).*/\1/
						s/[[:blank:]]*$//
						/^$/d
						p
					}'))
			;;
			addservice | as | modifyservice | ms | removeservice | rs)
				opts=(${opts[@]}$(echo; LC_ALL=POSIX $ZYPPER -q ls | \
					sed -rn '/^[0-9]/{
						s/^[0-9]+[[:blank:]]*\|[[:blank:]]*([^|]+).*/\1/
						s/[[:blank:]]*$//
						/^$/d
						p
					}'))
			;;
			removelock | rl)
				opts=(${opts[@]}$(echo; LC_ALL=POSIX $ZYPPER -q ll | \
					sed -rn '/^[0-9]/{
						s/^[0-9]+[[:blank:]]*\|[[:blank:]]*([^|]+).*/\1/p
						s/[[:blank:]]*$//
						/^$/d
						p
					}'))
			;;
		esac
		IFS=$'\n'
		COMPREPLY=($(compgen -W "${opts[*]}" -- ${cur}))
		_strip
	fi
	eval $noglob
}

complete -F _zypper -o default zypper
