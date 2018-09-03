/*
 * Copyright (C) agile6v
 */

#include "pupa.h"
#include "pupa_config.h"

#define JAVA_TRUE    1
#define JAVA_FALSE   0


JNIEXPORT jboolean JNICALL Java_PUPA_init
        (JNIEnv *env, jobject obj, jstring str, jint key_count, jint op_type)
{
    int         ret;
    const char *path = (*env)->GetStringUTFChars(env, str, 0);

    ret = pupa_init(path, key_count, op_type);
    if (ret != PUPA_OK) {
        return JAVA_FALSE;
    }

    return JAVA_TRUE;
}


JNIEXPORT jboolean JNICALL Java_PUPA_fini
        (JNIEnv *env, jobject obj)
{
    int ret;

    ret = pupa_fini();
    if (ret != PUPA_OK) {
        return JAVA_FALSE;
    }

    return JAVA_TRUE;
}


JNIEXPORT jboolean JNICALL Java_PUPA_set
        (JNIEnv *env, jobject obj, jstring key_str, jstring value_str)
{
    int         ret;
    pupa_str_t  key, value;

    key.data = (*env)->GetStringUTFChars(env, key_str, 0);
    value.data = (*env)->GetStringUTFChars(env, value_str, 0);

    key.len = strlen(key.data);
    value.len = strlen(value.data);

    ret = pupa_set(&key, &value);
    if (ret != PUPA_OK) {
        return JAVA_FALSE;
    }

    return JAVA_TRUE;
}


JNIEXPORT jboolean JNICALL Java_PUPA_delete
        (JNIEnv *env, jobject obj, jstring key_str)
{
    int         ret;
    pupa_str_t  key;

    key.data = (*env)->GetStringUTFChars(env, key_str, 0);
    key.len = strlen(key.data);

    ret = pupa_del(&key);
    if (ret != PUPA_OK) {
        return JAVA_FALSE;
    }

    return JAVA_TRUE;
}


JNIEXPORT jstring JNICALL Java_PUPA_get
        (JNIEnv *env, jobject obj, jstring key_str)
{
    int         ret;
    pupa_str_t  key, value;

    key.data = (*env)->GetStringUTFChars(env, key_str, 0);
    key.len = strlen(key.data);

    ret = pupa_get(&key, &value);
    if (ret != PUPA_OK) {
        return (*env)->NewStringUTF(env, "");
    }

    return (*env)->NewStringUTF(env, value.data);
}


JNIEXPORT jstring JNICALL Java_PUPA_stats
        (JNIEnv *env, jobject obj)
{
    int         ret;
    pupa_str_t  stat;

    ret = pupa_stats(&stat);
    if (ret != PUPA_OK) {
        return (*env)->NewStringUTF(env, "");
    }

    return (*env)->NewStringUTF(env, stat.data);
}