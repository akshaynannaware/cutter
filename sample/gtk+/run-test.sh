#!/bin/sh

export BASE_DIR="`dirname $0`"

if test x"$NO_MAKE" != x"yes"; then
    make -C $BASE_DIR/../../ > /dev/null || exit 1
fi

export CUT_UI_MODULE_DIR=$BASE_DIR/../../cutter/module/ui/.libs
export CUT_UI_FACTORY_MODULE_DIR=$BASE_DIR/../../cutter/module/ui/.libs
export CUT_REPORT_MODULE_DIR=$BASE_DIR/../../cutter/module/report/.libs
export CUT_REPORT_FACTORY_MODULE_DIR=$BASE_DIR/../../cutter/module/report/.libs

CUTTER=$BASE_DIR/cutter-gtk
if test x"$CUTTER_DEBUG" = x"yes"; then
    CUTTER="$BASE_DIR/../../libtool --mode=execute gdb --args $CUTTER"
fi
CUTTER_ARGS="-s $BASE_DIR"
if test x"$USE_GTK" = x"yes"; then
    CUTTER_ARGS="-u gtk $CUTTER_ARGS"
fi
$CUTTER $CUTTER_ARGS "$@" $BASE_DIR
