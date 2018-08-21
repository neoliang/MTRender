LOCAL_PATH			:= $(call my-dir)
SRC_PATH			:= ../..
COMMON_PATH			:= $(SRC_PATH)/../Common
COMMON_INC_PATH		:= $(COMMON_PATH)/Include \
						$(SRC_PATH)/../glm \
						$(SRC_PATH)/../rapidjson/include
COMMON_SRC_PATH		:= $(COMMON_PATH)/Source

include $(CLEAR_VARS)

LOCAL_MODULE    := DemoReturnIM
LOCAL_CFLAGS    += -DANDROID
LOCAL_CFLAGS	+= -DGLM_ENABLE_EXPERIMENTAL


LOCAL_SRC_FILES := $(COMMON_SRC_PATH)/esShader.cpp \
				   $(COMMON_SRC_PATH)/RingBuffer.cpp \
				   $(COMMON_SRC_PATH)/Mesh.cpp \
				   $(COMMON_SRC_PATH)/esShapes.cpp \
				   $(COMMON_SRC_PATH)/esTransform.cpp \
				   $(COMMON_SRC_PATH)/esUtil.cpp \
				   $(COMMON_SRC_PATH)/Android/esUtil_Android.cpp \
				   $(COMMON_SRC_PATH)/ThreadBufferESDevice.cpp \
				   $(COMMON_SRC_PATH)/ThreadESDeviceBase.cpp \
				   $(COMMON_SRC_PATH)/ESDevice.cpp \
				   $(COMMON_SRC_PATH)/ThreadESDevice.cpp \
				   $(COMMON_SRC_PATH)/DemoBase.cpp \
				   $(SRC_PATH)/DemoReturnIM.cpp
				   
				   
				   

LOCAL_C_INCLUDES	:= $(SRC_PATH) \
					   $(COMMON_INC_PATH)
				   
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv3

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
