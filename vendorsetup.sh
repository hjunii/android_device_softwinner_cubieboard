#
# Copyright (C) 2008 The Android Open Source Project
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

# This file is executed by build/envsetup.sh, and can use anything
# defined in envsetup.sh.
#
# In particular, you can add lunch options with the add_lunch_combo
# function: add_lunch_combo generic-eng

add_lunch_combo omni_cubieboard-userdebug

echo ""
echo "Applying patches for cubieboard"
echo ""

echo "Apply patch to external/libusb"
echo -n "Apply patch 0001-Make-the-pregenerated-config.h-adapt-to-linux-darwin.patch"
(cd external/libusb; git am ../../device/softwinner/cubieboard/patches/0001-Make-the-pregenerated-config.h-adapt-to-linux-darwin.patch) > /dev/null 2>&1
if [ $? == 0 ]; then
echo "     [DONE]"
else
(cd external/libusb; git am --abort)
echo "     [FAIL]"
fi
echo -n "Apply patch 0002-Add-libusb-support.patch"
(cd external/libusb; git am ../../device/softwinner/cubieboard/patches/0002-Add-libusb-support.patch) > /dev/null 2>&1
if [ $? == 0 ]; then
echo "     [DONE]"
else
(cd external/libusb; git am --abort)
echo "     [FAIL]"
fi

echo "Apply patch to hardware/broadcom/libbt"
echo -n "Apply patch 0001-libbt-BT-USB-support.patch"
(cd hardware/broadcom/libbt; git am ../../../device/softwinner/cubieboard/patches/0001-libbt-BT-USB-support.patch) > /dev/null 2>&1
if [ $? == 0 ]; then
echo "     [DONE]"
else
(cd hardware/broadcom/libbt; git am --abort)
echo "     [FAIL]"
fi

echo "Apply patch to external/bluetooth/bluedroid"
echo -n "Apply patch 0001-Support-USB-HCI.patch"
(cd external/bluetooth/bluedroid; git am ../../../device/softwinner/cubieboard/patches/0001-Support-USB-HCI.patch) > /dev/null 2>&1
if [ $? == 0 ]; then
echo "     [DONE]"
else
(cd external/bluetooth/bluedroid; git am --abort)
echo "     [FAIL]"
fi
echo -n "Apply patch 0002-Update-USB-support-for-10.2.patch"
(cd external/bluetooth/bluedroid; git am ../../../device/softwinner/cubieboard/patches/0002-Update-USB-support-for-10.2.patch) > /dev/null 2>&1
if [ $? == 0 ]; then
echo "     [DONE]"
else
(cd external/bluetooth/bluedroid; git am --abort)
echo "     [FAIL]"
fi
echo -n "Apply patch 0003-bta_hh-Change-bus_type-to-BUS_USB-for-USB-HCI.patch"
(cd external/bluetooth/bluedroid; git am ../../../device/softwinner/cubieboard/patches/0003-bta_hh-Change-bus_type-to-BUS_USB-for-USB-HCI.patch) > /dev/null 2>&1
if [ $? == 0 ]; then
echo "     [DONE]"
else
(cd external/bluetooth/bluedroid; git am --abort)
echo "     [FAIL]"
fi
