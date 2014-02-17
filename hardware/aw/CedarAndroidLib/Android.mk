LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := libcedarxosal.so libcedarv.so libcedarxbase.so libcedarv_base.so libcedarv_adapter.so libve.so libsunxi_alloc.so libaw_h264enc.so

LOCAL_MODULE_TAGS := optional

include $(BUILD_MULTI_PREBUILT)
