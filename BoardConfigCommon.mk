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
# This file sets variables that control the way modules are built
# thorughout the system. It should not be used to conditionally
# disable makefiles (the proper mechanism to control what gets
# included in a build is to use PRODUCT_PACKAGES in a product
# definition file).
#

# inherit from the proprietary version
# needed for BP-flashing updater extensions

TARGET_BOARD_PLATFORM := sun4i

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
ARCH_ARM_HAVE_TLS_REGISTER := true

TARGET_USERIMAGES_USE_EXT4 := true

BOARD_SYSTEMIMAGE_PARTITION_SIZE := 524288000
BOARD_USERDATAIMAGE_PARTITION_SIZE := 314572800
BOARD_FLASH_BLOCK_SIZE := 4096

# Build kernel from source
TARGET_KERNEL_SOURCE := kernel/softwinner/common
TARGET_KERNEL_CONFIG := cyanogenmod_cubieboard_defconfig

# kernel
BOARD_KERNEL_CMDLINE := init=/init loglevel=8
BOARD_KERNEL_BASE := 0x40000000
BOARD_PAGE_SIZE := 4096

# Wifi related defines
BOARD_WIFI_VENDOR            		:= realtek
WPA_SUPPLICANT_VERSION      		:= VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER 		:= NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB 	:= lib_driver_cmd_rtl
BOARD_HOSTAPD_DRIVER        		:= NL80211
BOARD_HOSTAPD_PRIVATE_LIB   		:= lib_driver_cmd_rtl
BOARD_WLAN_DEVICE 					:= rtl8192cu
WIFI_DRIVER_MODULE_PATH     		:= "/system/lib/modules/8192cu.ko"
WIFI_DRIVER_MODULE_NAME				:= "8192cu"
WIFI_DRIVER_MODULE_ARG				:= "ifname=wlan0"

TARGET_BOOTLOADER_BOARD_NAME := crane
TARGET_NO_BOOTLOADER := true

#BOARD_USES_GENERIC_AUDIO := true
#HAVE_HTC_AUDIO_DRIVER := true

BOARD_USES_GENERIC_INVENSENSE := false

#BOARD_HAVE_BLUETOOTH := true
#BOARD_HAVE_BLUETOOTH_BCM := true

USE_OPENGL_RENDERER := true
BOARD_EGL_CFG := device/softwinner/cubieboard/egl.cfg
BOARD_EGL_NEEDS_LEGACY_FB := true

# Avoid the generation of ldrcc instructions
NEED_WORKAROUND_CORTEX_A9_745320 := true

# Required for CWM
BOARD_HAS_NO_SELECT_BUTTON := true

BOARD_VOLD_MAX_PARTITIONS := 20
BOARD_VOLD_DISC_HAS_MULTIPLE_MAJORS := true

CEDARX_CHIP_VERSION := F23
