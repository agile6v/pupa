#!/usr/bin/env bash

echo=/bin/echo

$echo -n "checking for Go ..."

if /bin/sh -c "go version" >> /dev/null 2>&1; then
    $echo " found"
    $echo " + `go version`"
else
    $echo
    $echo $0: error: no Go found.
    $echo
    exit 1;
fi

GO_PATH=${GO_PATH=`go env GOPATH`}
GO_PATH=${GO_PATH:-`pwd`/go}

$echo " + Go package path: \"${GO_PATH}\""

install -d ${GO_PATH}/src/github.com/agile6v/pupa
install -p -m644 pupa.go ${GO_PATH}/src/github.com/agile6v/pupa

$echo "Install successfully."