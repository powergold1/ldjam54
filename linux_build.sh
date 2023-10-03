#!/bin/sh
cd src
sh linux_build.sh $@
rc=$?
cd ..
exit $rc
