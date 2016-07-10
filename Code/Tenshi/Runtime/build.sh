#!/bin/sh

CFLAGS="-W -Wall -pedantic -std=gnu99 -c"
CFLAGS_DEBUG="-g -D_DEBUG -DTRACE_ENABLED=0"

RTBINDIR="../../../Build/Bin64"

gcc $CFLAGS $CFLAGS_DEBUG -o "$RTBINDIR/TenshiRuntimeDbg.o" TenshiRuntime.c && \
gcc $CFLAGS -o "$RTBINDIR/TenshiRuntime.o" TenshiRuntime.c
