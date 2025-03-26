#!/bin/bash
#set -x


DESTROOTFS=/media/lukasz/rootfs
DESTBOOTFS=/media/lukasz/bootfs
#KERNEL=kernel7l
KERNEL=kernel_2712
#KERNEL=kernel7l
LOGFILE=$(basename $0).log
#CCPREFIX=arm-linux-gnueabihf-
CCPREFIX=aarch64-linux-gnu-
#CCPREFIX=arm-linux-gnueabihf-
#TOOLCHAINPATH=../tools/arm-bcm2708/arm-bcm2708-linux-gnueabi
TOOLCHAINPATH=/usr/bin
LOGONLY=1
SHOWALL=$2

function _runcmd()
{
	local CMD=$1
	local OUT=$2

	echo $CMD
	if [ -n "$CMD" ]; then
		echo ">> $CMD"
		sleep 0.1

		if [ -n "$OUT" -a $OUT -eq $LOGONLY ]; then $CMD 2>&1 >> ./$LOGFILE;
		else $CMD; fi
	fi
}


export PATH=$TOOLCHAINPATH/bin:$PATH
if [ -e $LOGFILE ]; then rm -f $LOGFILE &> /dev/null; fi

echo "to reset config run: make ARCH=arm64 bcm2712_defconfig"
read wait

case $1 in
	menu)   
		_runcmd "make ARCH=arm64 CROSS_COMPILE=$CCPREFIX menuconfig" $SHOWALL
		;;
	#build)   _runcmd "make -j30 ARCH=arm64 CROSS_COMPILE=$CCPREFIX zImage modules dtbs" $SHOWALL;;
	build)   _runcmd "make -j30 ARCH=arm64 CROSS_COMPILE=$CCPREFIX Image modules dtbs" $SHOWALL;;
	install) if [ "$EUID" -eq 0 ]; then
                     _runcmd "env PATH=$PATH make ARCH=arm64 CROSS_COMPILE=$CCPREFIX INSTALL_MOD_PATH=$DESTROOTFS modules_install" $LOGONLY
                     _runcmd "cp arch/arm64/boot/Image $DESTBOOTFS/$KERNEL.img" $LOGONLY
                     _runcmd "cp arch/arm64/boot/dts/overlays/*.dtb* $DESTBOOTFS/overlays/" $LOGONLY
                     _runcmd "cp arch/arm64/boot/dts/overlays/README $DESTBOOTFS/overlays/" $LOGONLY
                     _runcmd "cp arch/arm64/boot/dts/broadcom/*.dtb* $DESTBOOTFS/" $LOGONLY
                     _runcmd "cp arch/arm64/boot/dts/*.dtb $DESTBOOTFS/" $LOGONLY
                     _runcmd "umount $DESTBOOTFS $DESTROOTFS" $LOGONLY
                 else
                     echo "Run as root!";
                 fi;;
	*) echo "Invalid parameter '$1'!";;
esac


