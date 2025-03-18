#!/bin/bash

TRIGGERNUM=$1
TRIGGERSYSFS=/sys/bus/iio/devices/iio_sysfs_trigger
TRIGGEROS=$TRIGGERSYSFS/trigger$TRIGGERNUM/trigger_now

function _log()
{
	TAG=$1
	MSG=$2

	echo "[$TAG] $MSG"
}

function _help()
{
	echo -e "Script usage:\n./$(basename $0) <sysfs_trigger_num>"
	exit 1
}

function _istrigready()
{
	if [ ! -e $TRIGGEROS ]; then
		_log ERR "$TRIGGEROS not present"
	else
		_log INF "Sysfs trigger is ready to proceed"
		return 1;
	fi
	return 0
}

function _setuptrigger()
{
	_log INF "Updating sysfs trigger permissions"
	sudo chmod a+rw $TRIGGEROS
	if [ $? -ne 0 ]; then _log ERR "Cannot update $TRIGGEROS"; return 1; fi
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
if [ -z "$TRIGGERNUM" ]; then _help; fi
_istrigready
if [ $? -ne 1 ]; then exit 2; fi
_setuptrigger
if [ $? -ne 0 ]; then exit 3; fi

## end ##

