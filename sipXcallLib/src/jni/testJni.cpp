#include <string.h>
#include <jni.h>
#include <os/OsDateTime.h>
#include <os/OsTime.h>
#include <utl/UtlString.h>
#include <tapi/sipXtapi.h>

extern "C"

jstring Java_com_sipez_sipxjnitest_SipxJniTest_getHttpTime(JNIEnv* env,
                                                           jobject jthis)
{
    OsDateTime dateTimeNow;
    OsDateTime::getCurTime(dateTimeNow);
    UtlString dateString;
    dateTimeNow.getHttpTimeString(dateString);

    // Get the instance id member of the Java this object
    jclass jSipxJniTestClass = env->GetObjectClass(jthis);
    jfieldID mInsanceIdFieldId = env->GetFieldID(jSipxJniTestClass, "mInstanceId", "I");
    jint jmInstanceId = env->GetIntField(jthis, mInsanceIdFieldId);

    dateString.appendFormat("\nmInstanceId: %d", jmInstanceId);

    return (env->NewStringUTF(dateString));
}

jint Java_com_sipez_sipxjnitest_SipxJniTest_sipxInitialize(JNIEnv* env,
                                                           jobject jthis)
{
    SIPX_INST instance = NULL;
    int sipPort = 5060;
    int sipTlsPort = -1;
    int flowgraphSampleRate = 16000;
    int res = 0;
    // Initialize our sipXtapi handle.
    res = sipxInitialize(&instance,
                         sipPort,
                         sipPort,
                         sipTlsPort,
                         DEFAULT_RTP_START_PORT,
                         DEFAULT_CONNECTIONS,
                         DEFAULT_IDENTITY,
                         DEFAULT_BIND_ADDRESS,
                         false, // use sequential RTP/RTCP ports
                         NULL, // cert. nickname
                         NULL, // cert. password
                         NULL, // DB location
                         false, // Disable local audio
                         flowgraphSampleRate,
                         48000); // Audio device sample rate (not used in server)
    return((jint)instance);
}

// For arguments passed in we need to get and then release to tell
// Java we are done accessing the object
// e.g:
// const char *str = env->GetStringUTFChars(stringArg, 0); 
//
//     ... 
//
// env->ReleaseStringUTFChars(stringArg, str)

