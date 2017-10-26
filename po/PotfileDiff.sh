#! /bin/bash
set -e
function extract() { grep '^\(msgid\|"\)' "$1" | grep -v "POT-Creation-Date" | sort -u; }

OLDTMP=$(mktemp)
NEWTMP=$(mktemp)
trap "/bin/rm -f -- \"$OLDTMP\" \"$NEWTMP\"" 0 1 2 3 13 15

extract "$1" >"$OLDTMP"
extract "$2" >"$NEWTMP"

shift 2	# additional args for diff
diff -u0 "$@" "$OLDTMP" "$NEWTMP"
