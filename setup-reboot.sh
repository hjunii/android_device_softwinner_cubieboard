#!/system/bin/sh

echo -n boot-$1 | busybox dd of=/dev/block/mmcblk0p5 count=1 conv=sync; sync
