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
    jmethodID gifLoadFinishId = env->GetMethodID(clazz,"gifLoadFinish","(JII)V");
    jmethodID saveGifParserAddressId = env->GetMethodID(clazz,"saveGifParserAddress","(J)V");
    jint w = gifParser->getW();
    jint h = gifParser->getH();
    LOGD("screen size : %d  %d",w,h);
    env->CallVoidMethod(obj,gifLoadFinishId,gifParser->getIddSize(),w,h);
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
    LOGD("signature %s\n",gifParser->getSignature());
    OneFrame *oneFrame = gifParser->seekIddsTo(iddPosition);
    unsigned int transparentColor = gifParser->getGCT()->getColorIntAt(oneFrame->graphicControlExt->getTransparentInde());
    oneFrame->graphicControlExt->dump();
    unsigned int top = oneFrame->iddStruct->imageDescriptor->getTop();
    unsigned int left = oneFrame->iddStruct->imageDescriptor->getLeft();
    unsigned int w = oneFrame->iddStruct->imageDescriptor->getW();
    unsigned int h =oneFrame->iddStruct->imageDescriptor->getH();
    LOGD("top %d,left %d,w %d,h %d\n",top,left,w,h);
    jsize bitmapSise = (jsize)(oneFrame->iddStruct->bitmapArray->size());
    LOGD("bitmap size = %d\n",bitmapSise);
    jintArray bitmapArray = env->NewIntArray(bitmapSise);
    jint *carr = env->GetIntArrayElements(bitmapArray,NULL);
    for(int i = 0;i<bitmapSise;i++){
        carr[i] = oneFrame->iddStruct->bitmapArray->at(i);
    }
    env->ReleaseIntArrayElements(bitmapArray,carr,0);
    for(int i = 0;i<bitmapSise;i++){
        //LOGD("    color = %d",carr[i]);
    }
    jclass clazz = env->GetObjectClass(obj);
    jmethodID drawOneFrameId = env->GetMethodID(clazz,"drawOneFrame","(IIII[II)V");
    env->CallVoidMethod(obj,drawOneFrameId,top,left,w,h,bitmapArray,transparentColor);

}