LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE	:= droillo
LOCAL_SRC_FILES := droillo.cpp
LOCAL_LDLIBS 	:= -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue

$(call import-module, android/native_app_glue)

APP_PLATFORM := android-9