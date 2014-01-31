#
# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#ifeq ($(TARGET_PREBUILT_KERNEL),)
#LOCAL_KERNEL := device/softwinner/cubieboard/kernel
#else
#LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
#endif

PRODUCT_AAPT_CONFIG := normal large tvdpi hdpi
PRODUCT_AAPT_PREF_CONFIG := tvdpi


PRODUCT_PROPERTY_OVERRIDES := \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=15 \
    tf.enable=y \
    drm.service.enabled=true \
	audio.output.active=AUDIO_HDMI \
	audio.input.active=AUDIO_CODEC \

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp

include frameworks/native/build/tablet-7in-hdpi-1024-dalvik-heap.mk

PRODUCT_COPY_FILES += \
    device/softwinner/cubieboard/fstab.sun4i:root/fstab.sun4i \
    device/softwinner/cubieboard/ueventd.sun4i.rc:root/ueventd.sun4i.rc \
    device/softwinner/cubieboard/init.sun4i.usb.rc:root/init.sun4i.usb.rc \
    device/softwinner/cubieboard/gps.conf:system/etc/gps.conf

PRODUCT_COPY_FILES += \
	device/softwinner/cubieboard/tablet_core_hardware.xml:system/etc/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml
	#frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml 

PRODUCT_COPY_FILES += \
    device/softwinner/cubieboard/vold.fstab:system/etc/vold.fstab \
    device/softwinner/cubieboard/sun4i-keyboard.kl:system/usr/keylayout/sun4i-keyboard.kl \
    device/softwinner/cubieboard/sun4i-ir.kl:system/usr/keylayout/sun4i-ir.kl \
    device/softwinner/cubieboard/Generic.kl:system/usr/keylayout/Generic.kl \
    device/softwinner/cubieboard/sun4i-ts.idc:system/usr/idc/sun4i-ts.idc

PRODUCT_COPY_FILES += \
	device/softwinner/cubieboard/setup-reboot.sh:root/bin/setup-reboot.sh

PRODUCT_PACKAGES := \
	hwcomposer.sun4i \
	gralloc.sun4i \
	light.sun4i \
	audio.primary.sun4i \
    audio.a2dp.default \
    audio.usb.default \
    librs_jni \
    setup_fs \
    l2ping \
    hcitool \
    bttest \
    com.android.future.usb.accessory

PRODUCT_CHARACTERISTICS := tablet

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# media config xml file
PRODUCT_COPY_FILES += \
    device/softwinner/cubieboard/media_profiles.xml:system/etc/media_profiles.xml

# media codec config xml file
PRODUCT_COPY_FILES += \
    device/softwinner/cubieboard/media_codecs.xml:system/etc/media_codecs.xml

# Bluetooth config file
PRODUCT_COPY_FILES += \
    system/bluetooth/data/main.nonsmartphone.conf:system/etc/bluetooth/main.conf

# audio mixer paths
PRODUCT_COPY_FILES += \
    device/softwinner/cubieboard/mixer_paths.xml:system/etc/mixer_paths.xml

# audio policy configuration
PRODUCT_COPY_FILES += \
    device/softwinner/cubieboard/audio_policy.conf:system/etc/audio_policy.conf
