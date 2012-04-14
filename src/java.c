#include <allegro5/allegro.h>
#include <bass.h>

#include <jni.h>

static bool ok = false;

#define _jni_checkException(env) __jni_checkException(env, __FILE__, __FUNCTION__, __LINE__)
void __jni_checkException(JNIEnv *env, const char *file, const char *fname, int line);

#define _jni_call(env, rett, method, args...) ({ \
   rett ret = (*env)->method(env, ##args); \
   _jni_checkException(env); \
   ret; \
})

#define _jni_callv(env, method, args...) ({ \
   (*env)->method(env, ##args); \
   _jni_checkException(env); \
})

#define _jni_callVoidMethodV(env, obj, name, sig, args...) ({ \
   jclass class_id = _jni_call(env, jclass, GetObjectClass, obj); \
   \
   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig); \
   if(method_id == NULL) { \
   } else { \
      _jni_callv(env, CallVoidMethod, obj, method_id, ##args); \
   } \
   \
   _jni_callv(env, DeleteLocalRef, class_id); \
})

static jobject _jni_callObjectMethod(JNIEnv *env, jobject object, char *name, char *sig)
{
   jclass class_id = _jni_call(env, jclass, GetObjectClass, object);
   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig);
   jobject ret = _jni_call(env, jobject, CallObjectMethod, object, method_id);
   
   return ret;
}

jobject _al_android_activity_object();

ALLEGRO_DEBUG_CHANNEL("tmgCrap");

static JavaVM *javavm;
static JNIEnv *java_env;
static JNIEnv *thread_env;
static jclass bpc;

typedef struct FuncCall
{
	bool processed;
	const char *funcname;
	const char *filename;
	int handle;
	float vol, pan;
} FuncCall;

#define FUNCBUF_SIZE 250
static FuncCall funcs[FUNCBUF_SIZE];
static int insertPos = 0;
static int processPos = 0;
static ALLEGRO_MUTEX *mutex;
static bool need_return = false;
static int the_return;
static bool inited = false;

static void push(const char *funcname, const char *filename, int handle, float vol, float pan)
{
	al_lock_mutex(mutex);

	FuncCall f;
	f.processed = false;
	f.funcname = funcname;
	f.filename = filename;
	f.handle = handle;
	f.vol = vol;
	f.pan = pan;
	funcs[insertPos++] = f;
	insertPos %= FUNCBUF_SIZE;

	need_return = true;

	al_unlock_mutex(mutex);

	while (need_return);
}

static int wait(const char *funcname, const char *filename, int handle, float vol, float pan)
{
	push(funcname, filename, handle, vol, pan);
	return the_return;
}

static void *thread_proc(void *arg)
{
   	JavaVMAttachArgs attach_args = { JNI_VERSION_1_4, "java_audio_thread", NULL };
      	(*javavm)->AttachCurrentThread(javavm, &thread_env, &attach_args);
	
	jmethodID meth_initSound = (*thread_env)->GetStaticMethodID(thread_env, bpc, "initSound", "(Lorg/liballeg/app/AllegroActivity;)V");
	jmethodID meth_update = (*thread_env)->GetStaticMethodID(thread_env, bpc, "update", "()V");
	jmethodID meth_loadSample = (*thread_env)->GetStaticMethodID(thread_env, bpc, "loadSample", "(Ljava/lang/String;)I");
	jmethodID meth_loadSampleLoop = (*thread_env)->GetStaticMethodID(thread_env, bpc, "loadSampleLoop", "(Ljava/lang/String;)I");
	jmethodID meth_playSampleVolumePan = (*thread_env)->GetStaticMethodID(thread_env, bpc, "playSampleVolumePan", "(IFF)V");
	jmethodID meth_playSampleVolume = (*thread_env)->GetStaticMethodID(thread_env, bpc, "playSampleVolume", "(IF)V");
	jmethodID meth_playSample = (*thread_env)->GetStaticMethodID(thread_env, bpc, "playSample", "(I)V");
	jmethodID meth_stopSample = (*thread_env)->GetStaticMethodID(thread_env, bpc, "stopSample", "(I)V");
	jmethodID meth_loadMusic = (*thread_env)->GetStaticMethodID(thread_env, bpc, "loadMusic", "(Ljava/lang/String;)I");
	jmethodID meth_playMusic = (*thread_env)->GetStaticMethodID(thread_env, bpc, "playMusic", "(I)V");
	jmethodID meth_stopMusic = (*thread_env)->GetStaticMethodID(thread_env, bpc, "stopMusic", "(I)V");
	jmethodID meth_destroyMusic = (*thread_env)->GetStaticMethodID(thread_env, bpc, "destroyMusic", "(I)V");
	jmethodID meth_shutdownBASS = (*thread_env)->GetStaticMethodID(thread_env, bpc, "shutdownBASS", "()V");
	jmethodID meth_destroySample = (*thread_env)->GetStaticMethodID(thread_env, bpc, "destroySample", "(I)V");
	jmethodID meth_setMusicVolume = (*thread_env)->GetStaticMethodID(thread_env, bpc, "setMusicVolume", "(IF)V");

	ok = true;

	while (1) {
		al_lock_mutex(mutex);
		bool processed = funcs[processPos].processed;
		if (processed) {
			if (inited) {
				(*thread_env)->CallStaticVoidMethod(thread_env, bpc, meth_update);
			}
			al_unlock_mutex(mutex);
			if (inited) {
				al_rest(0.001);
			}
			continue;
		}
		const char *func = funcs[processPos].funcname;
		if (!strcmp(func, "initSound")) {
			(*thread_env)->CallStaticVoidMethod(thread_env, bpc, meth_initSound, _al_android_activity_object());
			ok = true;
		}
		else if (!strcmp(func, "shutdownBASS")) {
			(*thread_env)->CallStaticVoidMethod(thread_env, bpc, meth_shutdownBASS);
		}
		else if (!strcmp(func, "loadSample")) {
			jstring str = (*thread_env)->NewStringUTF(thread_env, funcs[processPos].filename);
			the_return = (*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_loadSample, str);
			(*thread_env)->DeleteLocalRef(thread_env, str);
		}
		else if (!strcmp(func, "loadSampleLoop")) {
			jstring str = (*thread_env)->NewStringUTF(thread_env, funcs[processPos].filename);
			the_return = (*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_loadSampleLoop, str);
			(*thread_env)->DeleteLocalRef(thread_env, str);
		}
		else if (!strcmp(func, "loadMusic")) {
			jstring str = (*thread_env)->NewStringUTF(thread_env, funcs[processPos].filename);
			the_return = (*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_loadMusic, str);
			(*thread_env)->DeleteLocalRef(thread_env, str);
		}
		else if (!strcmp(func, "playSampleVolumePan")) {
			(*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_playSampleVolumePan, funcs[processPos].handle, funcs[processPos].vol, funcs[processPos].pan);
		}
		else if (!strcmp(func, "playSampleVolume")) {
			(*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_playSampleVolume, funcs[processPos].handle, funcs[processPos].vol);
		}
		else if (!strcmp(func, "playSample")) {
			(*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_playSample, funcs[processPos].handle);
		}
		else if (!strcmp(func, "stopSample")) {
			(*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_stopSample, funcs[processPos].handle);
		}
		else if (!strcmp(func, "playMusic")) {
			(*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_playMusic, funcs[processPos].handle);
		}
		else if (!strcmp(func, "stopMusic")) {
			(*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_stopMusic, funcs[processPos].handle);
		}
		else if (!strcmp(func, "destroyMusic")) {
			(*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_destroyMusic, funcs[processPos].handle);
		}
		else if (!strcmp(func, "destroySample")) {
			(*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_destroySample, funcs[processPos].handle);
		}
		else if (!strcmp(func, "setMusicVolume")) {
			(*thread_env)->CallStaticIntMethod(thread_env, bpc, meth_setMusicVolume, funcs[processPos].handle, funcs[processPos].vol);
		}
		funcs[processPos].processed = true;
		processPos++;
		processPos %= FUNCBUF_SIZE;
		need_return = false;
		al_unlock_mutex(mutex);
	}

	return NULL;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	(void)reserved;

	javavm = vm;
	(*javavm)->GetEnv(javavm, (void **)&java_env, JNI_VERSION_1_4);

	jclass cls = (*java_env)->FindClass(java_env, "org/liballeg/app/BassPump");
	bpc = (*java_env)->NewGlobalRef(java_env, cls);
	
	return JNI_VERSION_1_4;
}

void bass_initSound(void)
{
	mutex = al_create_mutex();

	int i;
	for (i = 0; i < FUNCBUF_SIZE; i++) {
		funcs[i].processed = true;
	}

	al_run_detached_thread(thread_proc, NULL);

	push("initSound", NULL, 0, 0, 0);

	while (!ok) {
		al_rest(0.001);
	}

	inited = true;
}

HSAMPLE bass_loadSample(const char *name)
{
	return (HSAMPLE)wait("loadSample", name, 0, 0, 0);
}

HSAMPLE bass_loadSampleLoop(const char *name)
{
	return (HSAMPLE)wait("loadSampleLoop", name, 0, 0, 0);
}

void bass_playSampleVolumePan(HSAMPLE s, float vol, float pan)
{
	push("playSampleVolumePan", NULL, s, vol, pan);
}

void bass_playSampleVolume(HSAMPLE s, float vol)
{
	push("playSampleVolume", NULL, s, vol, 0);
}

void bass_playSample(HSAMPLE s)
{
	push("playSample", NULL, s, 0, 0);
}

void bass_stopSample(HSAMPLE s)
{
	push("stopSample", NULL, s, 0, 0);
}

HMUSIC bass_loadMusic(const char *name)
{
	return (HMUSIC)wait("loadMusic", name, 0, 0, 0);
}

void bass_playMusic(HMUSIC music)
{
	push("playMusic", NULL, music, 0, 0);
}

void bass_stopMusic(HMUSIC music)
{
	push("stopMusic", NULL, music, 0, 0);
}

void bass_destroyMusic(HMUSIC music)
{
	push("destroyMusic", NULL, music, 0, 0);
}

void bass_shutdownBASS(void)
{
	push("shutdownBASS", NULL, 0, 0, 0);
}

void bass_destroySample(HSAMPLE s)
{
	push("destroySample", NULL, s, 0, 0);
}

void bass_setMusicVolume(HMUSIC music, float vol)
{
	push("setMusicVolume", NULL, music, vol, 0);
}

void openURL(const char *url)
{
	jstring urlS = (*java_env)->NewStringUTF(java_env, url);

	_jni_callVoidMethodV(
		java_env,
		_al_android_activity_object(),
		"openURL",
		"(Ljava/lang/String;)V",
		urlS
	);
	
	(*java_env)->DeleteLocalRef(java_env, urlS);
}

// return true on success
bool get_clipboard(char *buf, int len)
{
	jstring s =
		(jstring)_jni_callObjectMethod(
			java_env,
			_al_android_activity_object(),
			"getClipData",
			"()Ljava/lang/String;"
		);
	
	if (s == NULL)
		return false;
	
	const char *native = (*java_env)->GetStringUTFChars(java_env, s, 0);

	strncpy(buf, native, len);

	(*java_env)->ReleaseStringUTFChars(java_env, s, native);

	return true;
}

void set_clipboard(char *buf)
{
	jstring saveS = (*java_env)->NewStringUTF(java_env, buf);

	_jni_callVoidMethodV(
		java_env,
		_al_android_activity_object(),
		"setClipData",
		"(Ljava/lang/String;)V",
		saveS
	);

	(*java_env)->DeleteLocalRef(java_env, saveS);
}