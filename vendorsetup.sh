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

add_lunch_combo cm_cubieboard-eng
add_lunch_combo cm_cubieboard-userdebug

echo "Apply patch to frameworks/native"
echo -n "Apply patch 0001-DisplayDevice-Backwards-compatibility-with-old-EGL.patch"
(cd frameworks/native; git am ../../device/softwinner/cubieboard/patches/0001-DisplayDevice-Backwards-compatibility-with-old-EGL.patch) > /dev/null 2>&1
if [ $? == 0 ]; then
    echo "     [DONE]"
else
    (cd frameworks/native; git am --abort)
    echo "     [FAIL]"
fi

echo "Apply patch to external/bluetooth/bluedroid"
echo -n "Apply patch 0001-bta_hh-Change-bus_type-to-BUS_USB-for-USB-HCI.patch"
(cd external/bluetooth/bluedroid; git am ../../../device/softwinner/cubieboard/patches/0001-bta_hh-Change-bus_type-to-BUS_USB-for-USB-HCI.patch) > /dev/null 2>&1
if [ $? == 0 ]; then
    echo "     [DONE]"
else
    (cd external/bluetooth/bluedroid; git am --abort)
    echo "     [FAIL]"
fi
