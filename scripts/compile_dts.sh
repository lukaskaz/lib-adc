#!/bin/bash

UNIT=$1
UNITDTS=${UNIT}.dts
UNITDTBO=${UNIT}.dtbo
COMPDTCMD="dtc -@ -I dts -O dtb -o /boot/overlays/$UNITDTBO $UNITDTS"

if [ -n "$UNIT" -a -e "$UNITDTS" ]; then
	echo $COMPDTCMD; $COMPDTCMD
	date
	ls -l /boot/overlays/$UNIT.dtbo
else
	echo "Invalid input file: $UNIT"
fi

