#!/bin/bash

DEVICE=$1
DEVICENODE=/dev/$DEVICE
SYSFSNODE=/sys/bus/iio/devices/$DEVICE

function _log()
{
	TAG=$1
	MSG=$2

	echo "[$TAG] $MSG"
}

function _help()
{
	echo -e "Script usage:\n./$(basename $0) <iio_device_name>"
	exit 1
}

function _isiiofsready()
{
	if [ ! -e $DEVICENODE ]; then
		_log ERR "$DEVICENODE not present"
	elif [ ! -e $SYSFSNODE ];then
		_log ERR "$SYSFSNODE not present"
	else
		_log INF "iio filesystem is ready to proceed"
		return 1;
	fi
	return 0
}

function _setupiiofs()
{
	_log INF "Updating iio filesystem permissions"
	sudo chmod a+rw $DEVICENODE
	if [ $? -ne 0 ]; then _log ERR "Cannot update $DEVICENODE"; return 1; fi
	sudo chmod -R a+rw $SYSFSNODE
	if [ $? -ne 0 ]; then _log ERR "Cannot update $SYSFSNODE"; return 1; fi
	return 0;	
}

function _onexit()
{
	RETCODE=$?
	if [ $RETCODE -ne 0 ]; then _log ERR "Setup failed"
	else _log INF "Setup successful"; fi
	exit $RETCODE
}

trap _onexit EXIT

## main ##
if [ -z "$DEVICE" ]; then _help; fi
_isiiofsready
if [ $? -ne 1 ]; then exit 2; fi
_setupiiofs
if [ $? -ne 0 ]; then exit 3; fi

## end ##

