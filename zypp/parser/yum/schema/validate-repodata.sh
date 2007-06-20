#!/bin/sh
USAGE=false
DO_xmllint=false
DO_jing_compact=false
DO_jing_xml=false
DO_rnv=false

#SDIR=/home/martin/zyppschema
SDIR=/home/mvidner/svn/zypp/trunk/libzypp/zypp/parser/yum/schema

while getopts ceghrs:x FLAG; do
    case $FLAG in
	c) DO_jing_compact=true;;
	e) set -o errexit;;
	g) DO_jing_xml=true;;
	h) USAGE=true;;
	r) DO_rnv=true;;
	s) SDIR="$OPTARG";;
	x) DO_xmllint=true;;
	*) USAGE=true; RC=1;;
    esac
done
shift $((OPTIND-1))

if $USAGE; then
    echo "Usage: $0 [options] <target>"
    echo " Target is a .../repodata directory or a single .xml(.gz) file"
    echo " Options:"
    echo "  -h  Help"
    echo
    echo "  -c  Use jing, rnc"
    echo "  -g  Use jing, rng"
    echo "  -r  Use RNV, rnc"
    echo "  -x  Use xmllint, rng (default)"
    echo
    echo "  -e  Exit on error"
    echo "  -s <dir> Directory with schemas"
    exit $RC
fi

$DO_jing_compact || $DO_jing_xml || $DO_rnv || $DO_xmllint || DO_xmllint=true


# val_foo:
# $1 schema basename w/o ext
# $2 file

RNV=/home/mvidner/tmp/rnv*/rnv
val_rnv() {
    $RNV $SDIR/$1.rnc $2	
}

val_jing_compact() {
    jing -c $SDIR/$1.rnc $2
}

val_jing_xml() {
    jing $SDIR/$1.rng $2
}

val_xmllint() {
    xmllint --noout --relaxng $SDIR/$1.rng $F
}

# $1 schema basename w/o ext
# $2 file or -
val() {
    test -f $2 || return 0

    echo \* $1
    F=$2
    TEMP=""
    case $F in
	*.gz)
	    TEMP=`mktemp /tmp/${F%.gz}.XXXXXX`
	    zcat $F > $TEMP
	    F=$TEMP
	    ;;
    esac

    FAIL=false
    for VALIDATOR in xmllint jing_compact jing_xml rnv; do
	COND_ref=DO_$VALIDATOR
	CALL=val_$VALIDATOR
	if ${!COND_ref}; then
	    echo \*\* $VALIDATOR
	    $CALL $1 $F || FAIL=true
	fi
    done

    rm -f $TEMP
    if $FAIL; then
	exit 1
    fi
}

# $1 file name
val_file() {
    case ${1##*/} in
	repomd.xml*|filelists.xml*|other.xml*|patches.xml*)
	    SCHEMA=${1%.xml*} ;;
	patch-*.xml)
	    SCHEMA=patch ;;
	primary.xml*)
	    SCHEMA=suse-primary ;;
	*)
	    echo "Do not know which schema to use for $1"
	    exit ;;
    esac
    val $SCHEMA $1
}

# main

if [ -f "$1" ]; then
    val_file "$1"
else
    if [ -d "$1" ]; then
	cd "$1"
    fi
    
    val_file repomd.xml
    val_file primary.xml.gz
    val_file filelists.xml.gz
    val_file other.xml.gz
    val_file patches.xml
    for p in patch-*.xml; do
	val_file $p
    done
fi
