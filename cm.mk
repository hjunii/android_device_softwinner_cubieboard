# Boot animation
TARGET_SCREEN_HEIGHT := 720
TARGET_SCREEN_WIDTH := 1280

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_full_tablet_wifionly.mk)

# Inherit device configuration
$(call inherit-product, device/softwinner/cubieboard/full_cubieboard.mk)

## Device identifier. This must come after all inclusions
PRODUCT_DEVICE := cubieboard
PRODUCT_NAME := cm_cubieboard
PRODUCT_BRAND := google
PRODUCT_MODEL := Nexus 7
PRODUCT_MANUFACTURER := asus

#Set build fingerprint / ID / Product Name ect.
PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=razor TARGET_DEVICE=flo BUILD_FINGERPRINT=google/razor/flo:4.4.3/KTU84L/1148727:user/release-keys PRIVATE_BUILD_DESC="razor-user 4.4.3 KTU84L 1148727 release-keys"
