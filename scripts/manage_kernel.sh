#!/bin/bash
#set -x


DESTROOTFS=/media/lukasz/rootfs
DESTBOOTFS=/media/lukasz/boot
KERNEL=kernel7l
LOGFILE=$(basename $0).log
CCPREFIX=arm-bcm2708-linux-gnueabi-
#TOOLCHAINPATH=../tools/arm-bcm2708/arm-bcm2708-linux-gnueabi
TOOLCHAINPATH=../tools/arm-bcm2708/arm-linux-gnueabihf
LOGONLY=1
SHOWALL=0

function _runcmd()
{
	local CMD=$1
	local OUT=$2

	if [ -n "$CMD" ]; then
		echo ">> $CMD"
		sleep 0.1

		if [ $OUT -eq $LOGONLY ]; then $CMD 2>&1 >> ./$LOGFILE;
		else $CMD; fi
	fi
}


export PATH=$TOOLCHAINPATH/bin:$PATH
if [ -e $LOGFILE ]; then rm -f $LOGFILE &> /dev/null; fi

case $1 in
	menu)    _runcmd "make ARCH=arm CROSS_COMPILE=$CCPREFIX menuconfig" $SHOWALL;;
	build)   _runcmd "make -j4 ARCH=arm CROSS_COMPILE=$CCPREFIX zImage modules dtbs" $SHOWALL;;
	install) if [ "$EUID" -eq 0 ]; then
                     _runcmd "env PATH=$PATH make ARCH=arm CROSS_COMPILE=$CCPREFIX INSTALL_MOD_PATH=$DESTROOTFS modules_install" $LOGONLY
                     _runcmd "cp arch/arm/boot/dts/overlays/*.dtb* $DESTBOOTFS/overlays/" $LOGONLY
                     _runcmd "cp arch/arm/boot/dts/overlays/README $DESTBOOTFS/overlays/" $LOGONLY
                     _runcmd "cp arch/arm/boot/dts/*.dtb $DESTBOOTFS/" $LOGONLY
                     _runcmd "cp arch/arm/boot/zImage $DESTBOOTFS/$KERNEL.img" $LOGONLY
                     _runcmd "umount $DESTBOOTFS $DESTROOTFS" $LOGONLY
                 else
                     echo "Run as root!";
                 fi;;
	*) echo "Invalid parameter '$1'!";;
esac


