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

# WARNING: This line must come *before* including the proprietary
# variant, so that it gets overwritten by the parent (which goes
# against the traditional rules of inheritance).
# The proprietary variant sets USE_CAMERA_STUB := false, this way
# we use the camera stub when the vendor tree isn't present, and
# the true camera library when the vendor tree is available.  Similarly,
# we set USE_PROPRIETARY_AUDIO_EXTENSIONS to true in the proprietary variant as
# well.

TARGET_BOOTLOADER_BOARD_NAME := sun4i
TARGET_BOOTLOADER_NAME := sun4i
TARGET_BOARD_INFO_FILE := device/softwinner/cubieboard/board-info.txt

BOARD_HAL_STATIC_LIBRARIES := libdumpstate.sun4i

# recovery
TARGET_RECOVERY_UI_LIB := librecovery_ui_cubieboard

TARGET_RELEASETOOLS_EXTENSIONS := device/softwinner/cubieboard

TARGET_RECOVERY_FSTAB = device/softwinner/cubieboard/recovery.fstab

-include vendor/softwinner/cubieboard/BoardConfigVendor.mk
include device/softwinner/cubieboard/BoardConfigCommon.mk
