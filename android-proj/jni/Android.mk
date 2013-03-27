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
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_memfile-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_memfile.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_primitives-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_primitives.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_image-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_image.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_font-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_font.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_ttf-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_ttf.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_color-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_color.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libbass-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libbass.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libbassflac-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libbassflac.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libbassmix-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libbassmix.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_ALLOW_UNDEFINED_SYMBOLS := false
LOCAL_MODULE    := monsterrpg2
LOCAL_SRC_FILES := \
	3d.cpp\
	Animation.cpp\
	AnimationSet.cpp\
	Area.cpp\
	atlas.cpp\
	atlas_accessor.cpp\
	battle.cpp\
	CombatActions.cpp\
	Combatant.cpp\
	CombatEntity.cpp\
	CombatEnemy.cpp\
	CombatPlayer.cpp\
	Configuration.cpp\
	debug.cpp\
	equipment.cpp\
	Frame.cpp\
	ftpget.cpp\
	GenericEffect.cpp\
	graphics.cpp\
	tftp_get.c\
	Image.cpp\
	Input.cpp\
	inventory.cpp\
	io.cpp\
	Items.cpp\
	java.c\
	lander.cpp\
	monster2.cpp\
	my_load_bitmap.cpp\
	Object.cpp\
	pause.cpp\
	Player.cpp\
	redundant.cpp\
	script.cpp\
	shooter.cpp\
	sound.cpp\
	sound-android.cpp\
	Spells.cpp\
	svg.cpp\
	tgui.cpp\
	Tile.cpp\
	translate.cpp\
	util.cpp\
	widgets.cpp\
	xml.cpp\
	init.cpp\

LOCAL_ARM_MODE := arm
LOCAL_CFLAGS    := -I$(ANDROID_NDK_TOOLCHAIN_ROOT)/user/armeabi/include -DDEBUGMODE -DA5_OGL -I$(ANDROID_NDK_TOOLCHAIN_ROOT)/sysroot/usr/include -I../include -DWITH_SVG -DOPENGLES

LOCAL_LDLIBS    := -L$(ANDROID_NDK_TOOLCHAIN_ROOT)/sysroot/usr/lib -L$(ANDROID_NDK_TOOLCHAIN_ROOT)/user/armeabi/lib -L$(LOCAL_PATH)/$(TARGET_ARCH_ABI) -llog libs/$(TARGET_ARCH_ABI)/liballegro.so libs/$(TARGET_ARCH_ABI)/liballegro_memfile.so libs/$(TARGET_ARCH_ABI)/liballegro_primitives.so libs/$(TARGET_ARCH_ABI)/liballegro_image.so libs/$(TARGET_ARCH_ABI)/liballegro_font.so libs/$(TARGET_ARCH_ABI)/liballegro_ttf.so libs/$(TARGET_ARCH_ABI)/liballegro_color.so -lGLESv1_CM -lstdc++ -llua5.2 -lz -lbass -lbassflac -lphysfs -lcurl -lsvgtiny -ldom -lexpat -lhubbub -lparserutils -lwapcaplet -lpng -ljpeg

include $(BUILD_SHARED_LIBRARY)

