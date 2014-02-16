#this file is used for Android compile configuration of AW_OMX

############################################################################
AW_OMX_EXT_CFLAGS :=

AW_OPENMAX_DEBUG_ENABLE := N
ifeq ($(AW_OPENMAX_DEBUG_ENABLE), Y)
AW_OPENMAX_DEBUG_CEDARV := Y
else
AW_OPENMAX_DEBUG_CEDARV := N
endif
ifeq ($(PLATFORM_VERSION),4.0.4)
AW_OMX_PLATFORM_VERSION := 6
AW_OMX_EXT_CFLAGS += -DAW_OMX_PLATFORM_VERSION=6
else
AW_OMX_PLATFORM_VERSION := 7
AW_OMX_EXT_CFLAGS += -DAW_OMX_PLATFORM_VERSION=7
endif
CEDARX_TOP := $(TOP)/device/softwinner/cubieboard/hardware/aw/CedarX
