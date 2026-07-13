#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DEPS=$DIR/../deps
SMT_SWITCH_VERSION=2133053735db6037d49f04a8adabba758a568eba

usage () {
    cat <<EOF
Usage: $0 [<option> ...]

Sets up the smt-switch API for interfacing with SMT solvers through a C++ API.

-h, --help              display this message and exit
--with-msat             include MathSAT which is under a custom non-BSD compliant license (default: off)
--python                build python bindings (default: off)
EOF
    exit 0
}

die () {
    echo "*** configure.sh: $*" 1>&2
    exit 1
}

WITH_MSAT=default
CONF_OPTS=""
WITH_PYTHON=default
cvc5_home=default

while [ $# -gt 0 ]
do
    case $1 in
        -h|--help) usage;;
        --with-msat)
            WITH_MSAT=ON
            CONF_OPTS="$CONF_OPTS --msat --msat-home=../mathsat";;
        --python)
            WITH_PYTHON=YES
            CONF_OPTS="$CONF_OPTS --python";;
        *) die "unexpected argument: $1";;
    esac
    shift
done

mkdir -p $DEPS

TOOLCHAIN_OPTS=""
BUNDLED_BISON="$DEPS/bison/bison-install/bin/bison"
BUNDLED_FLEX="$DEPS/flex/flex-install/bin/flex"
if [ -x "$BUNDLED_BISON" ] && "$BUNDLED_BISON" --version >/dev/null 2>&1; then
    TOOLCHAIN_OPTS="$TOOLCHAIN_OPTS --bison-dir=../bison/bison-install"
fi
if [ -x "$BUNDLED_FLEX" ] && "$BUNDLED_FLEX" --version >/dev/null 2>&1; then
    TOOLCHAIN_OPTS="$TOOLCHAIN_OPTS --flex-dir=../flex/flex-install"
fi

if [ ! -d "$DEPS/smt-switch" ]; then
    cd $DEPS
    git clone https://github.com/yangziyiiii/smt-switch.git
    cd smt-switch
    git checkout -f $SMT_SWITCH_VERSION
    ./contrib/setup-bitwuzla.sh
    ./contrib/setup-cvc5.sh
    CONF_OPTS="$CONF_OPTS --cvc5-home=$(pwd)/deps/cvc5"
    
    # pass bison/flex directories from smt-switch perspective
    ./configure.sh --bitwuzla --cvc5 $CONF_OPTS --prefix=local --static --smtlib-reader $TOOLCHAIN_OPTS
    cd build
    make -j$(nproc)
    # TODO put this back
    # temporarily disable due to test-disjointset issue
    # make test
    make install
    cd $DIR
else
    echo "$DEPS/smt-switch already exists. If you want to rebuild, please remove it manually."
fi

if [ -f "$DEPS/smt-switch/local/lib/libsmt-switch-bitwuzla.a" ]; then
    echo "It appears smt-switch with bitwuzla and cvc5 was successfully installed to $DEPS/smt-switch/local."
    echo "You may now build pono with: ./configure.sh && cd build && make"
else
    echo "Building smt-switch failed."
    echo "You might be missing some dependencies."
    echo "Please see the github page for installation instructions: https://github.com/makaimann/smt-switch"
    exit 1
fi
