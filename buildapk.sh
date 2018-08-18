echo dir $1 
echo buildinit $2

cd ./$1/Android

if [[ "$2" = "init" ]]
then
	echo 'init android project'
	android.bat update project -p . -t android-18
fi

cd jni/
#ndk-build clean
ndk-build
cd ../
ant debug
adb install -r bin/NativeActivity-debug.apk
cd ../../
