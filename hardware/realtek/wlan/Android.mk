ifeq ($(BOARD_WIFI_VENDOR), realtek)
	include $(call all-subdir-makefiles)
endif	

ifeq ($(strip $(WPA_SUPPLICANT_VERSION)),VER_0_8_X)
	include external/wpa_supplicant_8/wpa_supplicant/wpa_supplicant_conf.mk
else
	ifeq ($(strip $(WPA_SUPPLICANT_VERSION)),VER_0_6_X)
	include external/wpa_supplicant_6/wpa_supplicant/wpa_supplicant_conf.mk
else
	include external/wpa_supplicant/wpa_supplicant_conf.mk
endif
endif
