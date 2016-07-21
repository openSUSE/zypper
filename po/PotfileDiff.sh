#! /bin/bash
set -e
function extract() { grep '^\(msgid\|"\)' "$1" | grep -v "POT-Creation-Date" | sort -u; }

OLDTMP=$(mktemp)
trap " [ -f \"$OLDTMP\" ] && /bin/rm -f -- \"$OLDTMP\" " 0 1 2 3 13 15
extract "$1" >"$OLDTMP"

NEWTMP=$(mktemp)
trap " [ -f \"$NEWTMP\" ] && /bin/rm -f -- \"$NEWTMP\" " 0 1 2 3 13 15
extract "$2" >"$NEWTMP"

shift 2	# additional args for diff
diff -u0 "$@" "$OLDTMP" "$NEWTMP"
