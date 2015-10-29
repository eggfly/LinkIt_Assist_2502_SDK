# Copyright (C) 2009 The Android Open Source Project
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

LOCAL_PATH := $(call my-dir)
#GENERATE_ALL := $(shell test -f ./mtksrc/SmartConnection.c && echo yes)


# GENERATE_SMNT_LIB value options
# StaticLib- Create SmartConnection static library
# ShareLib - Create SmartConnection shared library
# ALL      - Create APK , and not Create SmartConnction Library
# nothing or other value  - Generate APK with prebuilt library
GENERATE_SMNT_LIB :=

ifeq ($(GENERATE_SMNT_LIB),StaticLib)
$(warning Generate smart connection static library!!)
include $(CLEAR_VARS)
LOCAL_MODULE:= libSmartConnection
LOCAL_SRC_FILES:= ../mtksrc/SmartConnection.c \
	../mtksrc/broadcastSSID.c  \
	../mtksrc/crypt_aes.c \
	../mtksrc/crypt_hmac.c \
	../mtksrc/crypt_sha2.c
LOCAL_MODULE_TAGS := optional
include $(BUILD_STATIC_LIBRARY)

else ifeq ($(GENERATE_SMNT_LIB),ShareLib)
$(warning Generate smart connection share library!!)
include $(CLEAR_VARS)
LOCAL_MODULE:= libSmartConnection
LOCAL_SRC_FILES:= ../mtksrc/SmartConnection.c \
	../mtksrc/broadcastSSID.c  \
	../mtksrc/crypt_aes.c \
	../mtksrc/crypt_hmac.c \
	../mtksrc/crypt_sha2.c
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

else ifeq ($(GENERATE_SMNT_LIB),ALL)
#ifeq ($(GENERATE_ALL),yes)
$(warning Generate IoTManager APK!!)
include $(CLEAR_VARS)
LOCAL_MODULE    := iotjni
LOCAL_SRC_FILES := iotjni.cpp \
	../mtksrc/broadcastSSID.c \
	../mtksrc/crypt_aes.c \
	../mtksrc/crypt_hmac.c \
	../mtksrc/crypt_sha2.c \
	../mtksrc/IoTControl.c 	\
	../mtksrc/IoTControlProtocol.c \
	../mtksrc/SmartConnection.c
LOCAL_LDLIBS := -llog
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
#endif

else
$(warning Generate IoTManager APK with prebuilt static library!!)

include $(CLEAR_VARS)
LOCAL_MODULE    := libSmartConnection
LOCAL_SRC_FILES := ../obj/local/$(TARGET_ARCH_ABI)/libSmartConnection.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := iotjni
LOCAL_SRC_FILES := iotjni.cpp \
	../mtksrc/IoTControl.c 	\
	../mtksrc/IoTControlProtocol.c \
	../mtksrc/crypt_aes.c \
	../mtksrc/crypt_hmac.c \
	../mtksrc/crypt_sha2.c
LOCAL_STATIC_LIBRARIES := libSmartConnection
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

endif