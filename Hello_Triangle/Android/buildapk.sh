cd jni/
#ndk-build clean
ndk-build
cd ..
ant debug
adb install -r bin/NativeActivity-debug.apk
