#
# Copyright 2012 The Android Open Source Project
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

# Sample: This is where we'd set a backup provider if we had one
# $(call inherit-product, device/sample/products/backup_overlay.mk)


# Inherit from the common Open Source product configuration
$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_base.mk)

# Inherit from our custom product configuration 
$(call inherit-product, vendor/omni/config/common_tablet.mk)

PRODUCT_NAME := omni_cubieboard
PRODUCT_DEVICE := cubieboard
PRODUCT_BRAND := Android
PRODUCT_MODEL := Nexus 7 2013
PRODUCT_MANUFACTURER := ASUS
PRODUCT_RESTRICT_VENDOR_FILES := false

# Inherit from hardware-specific part of the product configuration
$(call inherit-product, device/softwinner/cubieboard/device.mk)

#Set build fingerprint / ID / Product Name ect.
PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=razor TARGET_DEVICE=flo BUILD_FINGERPRINT=google/razor/flo:4.4.3/KTU84L/1148727:user/release-keys PRIVATE_BUILD_DESC="razor-user 4.4.3 KTU84L 1148727 release-keys"
