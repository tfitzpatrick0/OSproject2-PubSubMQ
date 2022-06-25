#!/bin/bash

FUNCTIONAL=test_queue_functional
WORKSPACE=/tmp/$FUNCTIONAL.$(id -u)
FAILURES=0

error() {
    echo "$@"
    [ -r $WORKSPACE/test ] && (echo; cat $WORKSPACE/test; echo)
    FAILURES=$((FAILURES + 1))
}

cleanup() {
    STATUS=${1:-$FAILURES}
    rm -fr $WORKSPACE
    exit $STATUS
}

mkdir $WORKSPACE

trap "cleanup" EXIT
trap "cleanup 1" INT TERM

echo
printf "%-40s ... " "Testing $FUNCTIONAL"

if [ ! -x bin/$FUNCTIONAL ]; then
    echo "Failure: bin/$FUNCTIONAL is not executable!"
    exit 1
fi

valgrind --leak-check=full bin/$FUNCTIONAL &> $WORKSPACE/test
if [ $? -ne 0 ] || [ $(awk '/ERROR SUMMARY:/ {print $4}' $WORKSPACE/test) -ne 0 ]; then
    error "Failure"
else
    echo "Success"
fi
