//
// Created by 宗开黎 on 2020-01-14.
//

#include <jni.h>
#include "JniCallbackHelper.h"
#include "macro.h"

JniCallbackHelper::JniCallbackHelper(JavaVM *javaVm, JNIEnv *env, jobject instance_) {
    this->javaVM = javaVm;
    this->env = env;
    //jobject 一旦涉及到跨方法、跨线程，需要创建全局引用
//    this->instance = instance;
    this->instance = env->NewGlobalRef(instance_);
    jclass clazz = env->GetObjectClass(this->instance);
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");
    jmd_onError = env->GetMethodID(clazz, "onError", "(I)V");
    jmd_onProgress = env->GetMethodID(clazz, "onProgress", "(I)V");
}

JniCallbackHelper::~JniCallbackHelper() {
    javaVM = 0;
    env->DeleteGlobalRef(instance);
    instance = 0;
    env = 0;
}

void JniCallbackHelper::onPrepared(int thread_mode) {
    //env不支持跨线程
    if (thread_mode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_prepared);
    } else {
        //子线程
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_prepared);
        javaVM->DetachCurrentThread();
    }
}

void JniCallbackHelper::onError(int thread_mode, int error_code) {
    if (thread_mode == THREAD_MAIN) {
        env->CallVoidMethod(instance, jmd_onError);
    } else {
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_onError, error_code);
        javaVM->DetachCurrentThread();
    }
}

void JniCallbackHelper::onProgress(int thread_mode, int progress) {
    if (thread_mode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_onProgress, progress);
    } else {
        //子线程
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_onProgress, progress);
        javaVM->DetachCurrentThread();
    }
}
