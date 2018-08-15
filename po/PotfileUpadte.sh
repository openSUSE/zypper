#! /bin/bash
set -e

function errexit() {
  echo "$0: $@" >&2
  exit 1
}

function currentBranch()
{ git rev-parse --abbrev-ref HEAD; }

test "$(currentBranch)" == "master"	|| errexit "Not on master branch."
test "$(basename "$PWD")" == "po"	|| errexit "Cd to the po directory."

test "$(git status --porcelain 2>/dev/null| grep '^[^ !?]' | wc -l)" == 0 || errexit "Index not clean: wont't commit"

BINDIR="${1}"
test -z "$BINDIR" && errexit "Missing argument: BINDIR"
test -d "$BINDIR" || errexit "Not a directory: BINDIR '$BINDIR'"
echo "Updating .pot .po files from $BINDIR..."
for F in *.pot *.po; do
  if [ -f "$BINDIR/$F" ]; then
    cp "$BINDIR/$F" .
  else
    errexit "Missing file $BINDIR/$F (touch CMakeLists.txt?)"
  fi
done

git add -A *.pot *.po
git commit -m 'Translation: updated .pot file'
