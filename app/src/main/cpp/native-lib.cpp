#include <jni.h>
#include <string>
#include <iostream>
#include "GIFParser.hpp"
#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"gif",__VA_ARGS__)

extern "C"
JNIEXPORT jstring JNICALL Java_com_zqlite_cpp_cppp_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */)
{
    std::string hello = "Hello from C++";
    jstring astring = env->NewStringUTF("aaaaaaaaaaaaaaaaa");

    return astring;
}


extern "C"
JNIEXPORT void JNICALL Java_com_zqlite_cpp_cppp_MainActivity_initGif(
        JNIEnv *env,
        jobject obj,
        jbyteArray gifbuffer,
        jint size){
    int len = env->GetArrayLength(gifbuffer);
    char *buffer = new char[len];
    env->GetByteArrayRegion(gifbuffer,0,len, reinterpret_cast<jbyte*>(buffer));
    __android_log_print(ANDROID_LOG_DEBUG,"gif"," gif size %d",len);

    GIFParser *gifParser = new GIFParser(buffer,size);
    gifParser->init();
    jclass clazz = env->GetObjectClass(obj);
    jmethodID gifLoadFinishId = env->GetMethodID(clazz,"gifLoadFinish","(J)V");
    jmethodID saveGifParserAddressId = env->GetMethodID(clazz,"saveGifParserAddress","(J)V");
    env->CallVoidMethod(obj,gifLoadFinishId,gifParser->getIddSize());
    env->CallVoidMethod(obj,saveGifParserAddressId,(long)gifParser);
}

extern "C"
JNIEXPORT void JNICALL Java_com_zqlite_cpp_cppp_MainActivity_seekIddTo(
        JNIEnv *env,
        jobject obj,
        jint iddPosition,
        jlong gifParserAddress
){
    GIFParser *gifParser = (GIFParser *)gifParserAddress;
    gifParser->seekIddsTo(iddPosition);
}