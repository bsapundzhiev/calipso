#APP_STL := gnustl_shared
#APP_ABI := armeabi armeabi-v7a
#APP_PLATFORM := android-10
APP_PROJECT_PATH :=.
APP_BUILD_SCRIPT :=$(APP_PROJECT_PATH)/Android.mk \
	$(APP_PROJECT_PATH)/mod_http/Android.mk\
	$(APP_PROJECT_PATH)/mod_autoind/Android.mk

