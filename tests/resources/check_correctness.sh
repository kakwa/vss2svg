#!/bin/sh

OUTDIR="../out"
vssDIR="./vss"
ret=0
VAGRIND_CMD="valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --error-exitcode=42"

VERBOSE=1
ABSPATH=$(readlink -f "$(dirname $0)")
STOPONERROR="no"

help(){
    cat <<EOF
usage: `basename $0` [-h] [-v] [-e <vss dir>] [-s] [-n] 

Script checking memleaks, segfault and svg correctness of vss2svg-conv

arguments:
  -h: diplays this help
  -v: verbose mode, print vss records
  -e: alternate vss dir (default '`readlink -f "$ABSPATH/$vssDIR"`')
  -s: stop on first error
  -r: resize to 800x600
  -x: disable xmllint check (svg integrity)
  -n: disable valgrind (memleaks checks)
  -N: ignore return code of vss2svg-conv (useful for checks on corrupted files)
EOF
    exit 1
}


while getopts ":hnNxrvse:" opt; do
  case $opt in

    h) 
        help
        ;;
    n)
        VAGRIND_CMD=""
        ;;
    v)
        VERBOSE=0
        VERBOSE_OPT='--verbose'
        ;;
    e)
        vssDIR=`readlink -f "$OPTARG" |sed "s%$ABSPATH%.%"`
        ;;
    s)
        STOPONERROR="yes"
        ;;
    x)
        XMLLINT="no"
        ;;
    r)
        RESIZE_OPTS="-w 800 -h 600"
        ;;
    N)
        IGNORECONVERR="yes"
        ;;
    \?)
        echo "Invalid option: -$OPTARG" >&2
        help
        exit 1
        ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
        help
        exit 1
        ;;
  esac
done

verbose_print(){
    msg="$1"
    if [ $VERBOSE -eq 0 ]
    then
       printf "$msg\n"
    fi
}

cd $ABSPATH
. ./colors.sh
rm -rf $OUTDIR
mkdir -p $OUTDIR
CMD="`readlink -f ../../vss2svg-conv`"
OUTDIR=`readlink -f $OUTDIR`
DTD=`readlink -f ./svg11-flat.dtd`
for vss in `find $vssDIR -type f -name "*.vss" |sort`
do
    vss="`readlink -f $vss`"
    SVG="${OUTDIR}/`basename ${vss}`"
    verbose_print "\n############## `basename "${vss}"` ####################"
    verbose_print "Command: $CMD $RESIZE_OPTS -i \"$vss\" -o \"${SVG}\""
    $VAGRIND_CMD $CMD $RESIZE_OPTS -i "$vss" -o ${SVG} $VERBOSE_OPT
    tmpret=$?
    if [ $tmpret -ne 0 ]
    then
        printf "[${BYel}ERROR${RCol}] vss2svg-conv exited on error or memleaked or crashed converting vss '$vss'\n"
        [ $tmpret -eq 42 ] || [ $tmpret -eq 139 ] || ! [ "$IGNORECONVERR" = "yes" ] && ret=1
    fi
    if ! [ "$XMLLINT" = "no" ]
    then
        for f in `ls ${SVG}/*`;
        do
            o=`readlink -f $f`
            xmllint --dtdvalid ./svg11-flat.dtd  --noout $o >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                printf "[${BYel}ERROR${RCol}] vss2svg-conv generate bad svg\n"
                printf "source vss:  $vss\n"
                printf "out svg   :  $SVG\n\n"
                printf "xmllint result:\n"
                xmllint --dtdvalid ./svg11-flat.dtd  --noout $o 2>&1 >/dev/null
                printf "\n"
                printf "Convert and xmllint commands:\n"
                printf "$CMD -i \"$vss\" -o \"${SVG}\"\n"
                printf "xmllint --dtdvalid $DTD --noout $o\n\n"
                ret=1
            fi
        done
    fi
    verbose_print "\n#####################################################\n"
    [ "${STOPONERROR}" = "yes" ] && [ $ret -eq 1 ] && exit 1
done
if [ $ret -ne 0 ]
then
    printf "[${BRed}FAIL${RCol}] Check exited with error(s)\n"
else
    printf "[${BGre}SUCCESS${RCol}] Check Ok\n"
fi
exit $ret
