#!/bin/sh
cc="g++"
cc="clang++"
debug=0

warn="-Werror -Wall -Wno-char-subscripts -Wno-unused-function -Wno-switch -Wno-sign-compare -Wno-unknown-pragmas"
if [ $cc = "g++" ] ; then
	diag="-fno-diagnostics-show-caret"
	warn="$warn -Wno-write-strings -Wno-class-memaccess"
else
	diag="-fno-caret-diagnostics"
	warn="$warn -Wno-writable-strings"
fi
app_def="-Dm_app -Dm_use_sdl_platform"
opt_debug="-std=c++17 -Isrc/ -g -Dm_debug"
opt_release="-std=c++17 -Isrc/ -O2 -DNDEBUG"
libs="-lSDL2 -lGL"

compile=0
debug=1

if [ $# -gt 0 ] ; then
	case $1 in
		"tags" )
			ctags src/*.cpp src/*.h
			;;
		"compile" )
			compile=1
			;;
		"clean" )
			rm -f client client.so
			;;
		"release" )
			compile=1
			debug=0
			;;
	esac
else
	compile=1
fi

if [ $compile = 1 ] ; then
	if [ $debug = 1 ] ; then
		opt=$opt_debug
	else
		opt=$opt_release
	fi
	echo $opt
	$cc $warn $diag $opt $app_def -c client.cpp -o client.o &
	$cc $warn $diag $opt $app_def -c sdl_platform.cpp -o sdl_platform.o &
	wait $(jobs -p)
	rc=$?
	if [ $rc != 0 ] ; then
		exit $rc
	fi
	$cc $warn $diag $opt $app_def client.o sdl_platform.o -o ../client $libs
	rc=$?
	if [ $rc != 0 ] ; then
		exit $rc
	fi
	if [ $debug = 0 ] ; then
		strip ../client
	fi
fi
