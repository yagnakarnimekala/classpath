/* GConfNativePeer.c -- Implements native methods for class GConfNativePeer
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   
   This file is part of GNU Classpath.
   
   GNU Classpath is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   GNU Classpath is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with GNU Classpath; see the file COPYING.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA.
   
   Linking this library statically or dynamically with other modules is
   making a combined work based on this library.  Thus, the terms and
   conditions of the GNU General Public License cover the whole
   combination.
   
   As a special exception, the copyright holders of this library give you
   permission to link this library with independent modules to produce an
   executable, regardless of the license terms of these independent
   modules, and to copy and distribute the resulting executable under
   terms of your choice, provided that you also meet, for each linked
   independent module, the terms and conditions of the license of that
   module.  An independent module is a module which is not derived from
   or based on this library.  If you modify this library, you may extend
   this exception to your version of the library, but you are not
   obligated to do so.  If you do not wish to do so, delete this
   exception statement from your version. */

#include <stdio.h>
#include <string.h>

#include <jni.h>

#include <glib.h>
#include <gconf/gconf-client.h>

#include "jcl.h"

#include "gnu_java_util_prefs_gconf_GConfNativePeer.h"

/*
 * Cached id, methods and objects
 */

/** Reference count */
static int reference_count = 0;

/** GConfClient backend */
static GConfClient *client = NULL;

/** java.util.ArrayList class */
static jclass jlist_class = NULL;

/** java.util.ArrayList constructor id */
static jmethodID jlist_init_id = NULL;

/** ava.util.ArrayList add id */
static jmethodID jlist_add_id = NULL;

/* ***** PRIVATE FUNCTIONS DELCARATION ***** */

/**
 * Gets the reference of the default GConfClient and initialize the
 * the type system.
 * The client reference should be released with g_object_unref after use.
 */
static void init_gconf_client (void);

/**
 * Throws a new runtime exception after a failure, with the given message.
 */
static void throw_exception (JNIEnv *env, const char *msg);

/**
 * Throws the given exception after a failure, with the given message.
 */
static void
throw_exception_by_name (JNIEnv *env, const char *name, const char *msg);

/**
 * Return a reference to a java.util.ArrayList class.
 */
static gboolean set_jlist_class (JNIEnv *env);

/**
 * Builds a new reference to a new java.util.ArrayList instace.
 * The instance should be freed by the caller after use.
 */
static jclass get_jlist_reference (JNIEnv *env, jclass jlist_class);

/* ***** END: PRIVATE FUNCTIONS DELCARATION ***** */

/* ***** NATIVE FUNCTIONS ***** */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    init_class
 * Signature: ()V
 */
JNIEXPORT void
JNICALL Java_gnu_java_util_prefs_gconf_GConfNativePeer_init_1class
	(JNIEnv *env, jclass clazz)
{
	if (reference_count == 0) {
		Java_gnu_java_util_prefs_gconf_GConfNativePeer_init_1id_1chache
				(env, clazz);
		return;
	}
	
	reference_count++;
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_init_1class */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    init_id_chache
 * Signature: ()V
 */
JNIEXPORT void
JNICALL Java_gnu_java_util_prefs_gconf_GConfNativePeer_init_1id_1cache
	(JNIEnv *env, jclass clazz __attribute__ ((unused)))
{
	reference_count++;

	init_gconf_client ();
	
	/* if client is null, there is probably an out or memory */
    if (client == NULL) {
    	/* release the string and throw a runtime exception */
       	throw_exception (env,
       		"Unable to initialize GConfClient in native code\n");
  		return;
   	}
   	
   	/* ***** java.util.ArrayList ***** */
   	if (set_jlist_class (env) == FALSE) {
   		throw_exception (env,
   			"Unable to get valid reference to java.util.List in native code\n");
   		return;
   	}
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_init_1id_1cache */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    gconf_client_gconf_client_all_keys
 * Signature: (Ljava/lang/String;)Ljava/util/List;
 */
JNIEXPORT jobject JNICALL
Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1gconf_1client_1all_1keys
	(JNIEnv *env, jclass clazz __attribute__ ((unused)), jstring node)
{
	const char 	*dir	 = NULL;
	GError 		*err 	 = NULL;
	GSList 		*entries = NULL;
	
	/* java.util.ArrayList */
	jobject jlist = NULL;

	dir = JCL_jstring_to_cstring(env, node);
	if (dir == NULL) {
		return NULL;
	}

    entries = gconf_client_all_entries (client, dir, &err);
    if (err != NULL) {
		throw_exception_by_name (env, "java/util/prefs/BackingStoreException",
								 err->message);								 
		g_error_free (err);
		err = NULL;
		
		JCL_free_cstring(env, node, dir);
		return NULL;
	}

	jlist = get_jlist_reference (env, jlist_class);
	if (jlist == NULL) {
		throw_exception_by_name (env, "java/util/prefs/BackingStoreException",
				"Unable to get java.util.List reference in native code\n");
		JCL_free_cstring(env, node, dir);
		g_slist_foreach (entries, (GFunc) gconf_entry_free, NULL);
		g_slist_free (entries);
		return NULL;
	}

	GSList* tmp = entries;
	while (tmp != NULL) {
		const char *_val = gconf_entry_get_key(tmp->data);		
		_val = strrchr (_val, '/');	++_val;
		(*env)->CallBooleanMethod(env, jlist, jlist_add_id,
								  (*env)->NewStringUTF(env, _val));
		tmp = g_slist_next (tmp);
	}

	/* clean up things */
	JCL_free_cstring(env, node, dir);
	g_slist_foreach (entries, (GFunc) gconf_entry_free, NULL);
	g_slist_free (entries);

	return jlist;
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1gconf_1client_1all_1keys */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    gconf_client_gconf_client_all_nodes
 * Signature: (Ljava/lang/String;)Ljava/util/List;
 */
JNIEXPORT jobject JNICALL
Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1gconf_1client_1all_1nodes
	(JNIEnv *env, jclass clazz __attribute__ ((unused)), jstring node)
{
	const char 	*dir	 = NULL;
	GError 		*err 	 = NULL;
	GSList 		*entries = NULL;
	
	/* java.util.ArrayList */
	jobject jlist = NULL;
	   
    dir = JCL_jstring_to_cstring(env, node);
	if (dir == NULL) {
		return NULL;
	}

	entries = gconf_client_all_dirs (client, dir, &err);
    if (err != NULL) {
		throw_exception_by_name (env, "java/util/prefs/BackingStoreException",
								 err->message);
		g_error_free (err);
		err = NULL;
		JCL_free_cstring(env, node, dir);
		return NULL;
	}

	jlist = get_jlist_reference (env, jlist_class);
	if (jlist == NULL) {
		throw_exception_by_name (env, "java/util/prefs/BackingStoreException",
				"Unable to get java.util.List reference in native code\n");
		JCL_free_cstring(env, node, dir);
		g_slist_foreach (entries, (GFunc) gconf_entry_free, NULL);
		g_slist_free (entries);
		return NULL;
	}

	GSList* tmp = entries;
	while (tmp != NULL) {
		const char *_val = tmp->data;
		_val = strrchr (_val, '/');	++_val;
		(*env)->CallBooleanMethod(env, jlist, jlist_add_id,
								  (*env)->NewStringUTF(env, _val));
		tmp = g_slist_next (tmp);
	}

	/* clean up things */
	JCL_free_cstring(env, node, dir);
	g_slist_foreach (entries, (GFunc) gconf_entry_free, NULL);
	g_slist_free (entries);

	return jlist;
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1gconf_1client_1all_1nodes */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    gconf_client_suggest_sync
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1suggest_1sync
	(JNIEnv *env, jclass clazz __attribute__ ((unused)))
{
	GError *err = NULL;
	
    gconf_client_suggest_sync (client, &err);
    if (err != NULL) {
		throw_exception_by_name (env, "java/util/prefs/BackingStoreException",
								 err->message);
		g_error_free (err);
		err = NULL;
	}
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1suggest_1sync */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    gconf_client_unset
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL
Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1unset
	(JNIEnv *env, jclass clazz __attribute__ ((unused)), jstring key)
{
    const char *_key	= NULL;
	gboolean	result	= JNI_FALSE;

	_key = JCL_jstring_to_cstring(env, key);
	if (_key == NULL) {
		return JNI_FALSE;
	}

	result = gconf_client_unset (client, _key, NULL);

	JCL_free_cstring(env, key, _key);

	return result;
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1unset */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    gconf_client_get_string
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1get_1string
	(JNIEnv *env, jclass clazz __attribute__ ((unused)), jstring key)
{
    const char *_key 	= NULL;
    const char *_value	= NULL;
	jstring	 	result	= NULL;
	
	_key = JCL_jstring_to_cstring(env, key);
	if (_key == NULL) {
		return NULL;
	}
	
	_value = gconf_client_get_string (client, _key, NULL);
	JCL_free_cstring(env, key, _key);

	result = (*env)->NewStringUTF (env, _value);
	g_free ((gpointer) _value);

	return result;
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1get_1string */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    gconf_client_set_string
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL
Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1set_1string
	(JNIEnv *env, jclass clazz __attribute__ ((unused)),
	 jstring key, jstring value)
{
    const char *_key	= NULL;
    const char *_value = NULL;
 	
 	gboolean result = JNI_FALSE;

    /* load an UTF string from the virtual machine. */
    _key   = JCL_jstring_to_cstring (env, key);
    _value = JCL_jstring_to_cstring (env, value);
 	if (_key == NULL && _value == NULL) {
		return JNI_FALSE;
	}
 
	result = gconf_client_set_string (client, _key, _value, NULL);

    JCL_free_cstring (env, key, _key);
    JCL_free_cstring (env, value, _value);
        
    return (jboolean) result;
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1set_1string */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    gconf_client_remove_dir
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1remove_1dir
	(JNIEnv *env, jclass clazz __attribute__ ((unused)), jstring node)
{
    const char *dir = NULL;
    
    dir = JCL_jstring_to_cstring (env, node);
	if (dir == NULL) {
		return NULL;
	}
	
	gconf_client_remove_dir (client, dir, NULL);
	
    JCL_free_cstring (env, node, dir);
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1remove_1dir */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    gconf_client_add_dir
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1add_1dir
	(JNIEnv *env, jclass clazz __attribute__ ((unused)), jstring node)
{
    const char *dir	= NULL;

    dir = JCL_jstring_to_cstring (env, node);
	if (dir == NULL) {
		return NULL;
	}
	
	/* ignore errors */
	gconf_client_add_dir (client, dir, GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);

    JCL_free_cstring (env, node, dir);
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1add_1dir */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    gconf_client_dir_exists
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL
Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1dir_1exists
    (JNIEnv *env, jclass clazz, jstring node)
{ 
    const char *dir	  = NULL;
	jboolean    value = JNI_FALSE;
	
    dir = JCL_jstring_to_cstring (env, node);
	if (dir == NULL) {
		return NULL;
	}
	
    /* we ignore errors here */
    value = gconf_client_dir_exists (client, dir, NULL);
	
	JCL_free_cstring (env, node, dir);

    return value;
}           /* Java_gnu_java_util_prefs_gconf_GConfNativePeer_gconf_1client_1dir_1exists */

/*
 * Class:     gnu_java_util_prefs_gconf_GConfNativePeer
 * Method:    finalize_class
 * Signature: ()V
 */
JNIEXPORT void
JNICALL Java_gnu_java_util_prefs_gconf_GConfNativePeer_finalize_1class
	(JNIEnv *env, jclass clazz)
{
	if (reference_count == 0) {
		/* last reference, free all resources and return */
		g_object_unref (G_OBJECT (client));
		
		(*env)->DeleteGlobalRef (env, jlist_class);
		
		jlist_class = NULL;
		jlist_init_id = NULL;
		jlist_add_id = NULL;
		
		return;
	}
	
	reference_count--;
}			/* Java_gnu_java_util_prefs_gconf_GConfNativePeer_finalize_1class */

/* ***** END: NATIVE FUNCTIONS ***** */

/* ***** PRIVATE FUNCTIONS IMPLEMENTATION ***** */

static void throw_exception (JNIEnv *env, const char *msg)
{
    throw_exception_by_name (env, "java/lang/RuntimeException", msg);
}			/* throw_exception */

static void
throw_exception_by_name (JNIEnv *env, const char *name, const char *msg)
{    
    JCL_ThrowException(env, name, msg);
}			/* throw_exception */

static void init_gconf_client (void)
{  
    client = gconf_client_get_default ();
    g_type_init();
}           /* init_gconf_client */

static gboolean set_jlist_class (JNIEnv *env)
{
	jclass local_jlist_class = NULL;

    /* gets a reference to the ArrayList class */
	local_jlist_class = JCL_FindClass (env, "java/util/ArrayList");
	if (local_jlist_class == NULL) {
		return FALSE;
	}
	
	jlist_class = (*env)->NewGlobalRef(env, local_jlist_class);
	(*env)->DeleteLocalRef(env, local_jlist_class);
	if (jlist_class == NULL) {
		return FALSE;
	}

	/* and initialize it */
	jlist_init_id = (*env)->GetMethodID (env, jlist_class, "<init>", "()V");
	if (jlist_init_id == NULL) {
		return FALSE;
	}

	jlist_add_id = (*env)->GetMethodID (env, jlist_class, "add",
										"(Ljava/lang/Object;)Z");
	if (jlist_add_id == NULL) {
		return FALSE;
	}

	return TRUE;
}		/* set_jlist_class */

static jobject get_jlist_reference (JNIEnv *env, jclass jlist_class)
{
	return (*env)->NewObject(env, jlist_class, jlist_init_id);
}			/* get_jlist_reference */

/* ***** END: PRIVATE FUNCTIONS IMPLEMENTATION ***** */