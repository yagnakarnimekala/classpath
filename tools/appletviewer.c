/* appletviewer.c -- a native appletviewer wrapper for VMs that
   support the JNI invocation interface
   Copyright (C) 2006  Free Software Foundation, Inc.

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

#include <jni.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

#ifndef JNI_VERSION_1_2
# error JNI version 1.2 or greater required
#endif

union env_union
{
  void *void_env;
  JNIEnv *jni_env;
};

int
main (int argc, const char** argv)
{
  union env_union tmp;
  JNIEnv* env;
  JavaVM* jvm;
  JavaVMInitArgs vm_args;
  jint result;
  jclass class_id;
  jmethodID method_id;
  jstring str;
  jclass string_class_id;
  jobjectArray args_array;
  char** non_vm_argv;
  int non_vm_argc;
  int i;
  int classpath_found = 0;

  env = NULL;
  jvm = NULL;

  vm_args.nOptions = 0;
  vm_args.options = NULL;

  non_vm_argc = 0;
  non_vm_argv = NULL;

  if (argc > 1)
    {
      for (i = 1; i < argc; i++)
	{
	  if (!strncmp (argv[i], "-J", 2))
	    {
	      if (!strncmp (argv[i], "-J-Djava.class.path=", 20))
		classpath_found = 1;

	      /* A virtual machine option. */
	      vm_args.options = (JavaVMOption*) realloc (vm_args.options, (vm_args.nOptions + 1) * sizeof (JavaVMOption));

	      if (vm_args.options == NULL)
		{
		  g_printerr ("appletviewer: realloc failed.\n");
		  goto destroy;
		}

	      if (strlen (argv[i]) == 2)
		{
		  g_printerr ("appletviewer: the -J option must not be followed by a space.\n");
		  goto destroy;
		}
	      else
		vm_args.options[vm_args.nOptions++].optionString = g_strdup (argv[i] + 2);
	    }
	  else if (!strncmp (argv[i], "--version", 9)
		   && argv[i][9] == '\0')
	    {
	      g_print ("appletviewer (GCJ Applet Viewer) " PACKAGE_VERSION "\n");
	      exit (0);
	    }
	  else if (!strncmp (argv[i], "-debug", 6)
		   && argv[i][6] == '\0')
	    {
	      /* FIXME: Ignore for now.  The debug option will be
		 unsupported until we have the ability to debug
		 bytecode.  For now, just strip it out of the argument
		 list. */
	    }
	  else
	    {
	      non_vm_argv = (char**) realloc (non_vm_argv, (non_vm_argc + 1) * sizeof (char*));
	      if (non_vm_argv == NULL)
		{
		  g_printerr ("appletviewer: realloc failed.\n");
		  goto destroy;
		}
	      non_vm_argv[non_vm_argc++] = g_strdup (argv[i]);
	    }
	}
    }

  if (!classpath_found)
    {
      /* Set the invocation classpath. */
      vm_args.options = (JavaVMOption*) realloc (vm_args.options, (vm_args.nOptions + 1) * sizeof (JavaVMOption));

      if (vm_args.options == NULL)
	{
	  g_printerr ("appletviewer: realloc failed.\n");
	  goto destroy;
	}

      vm_args.options[vm_args.nOptions++].optionString = "-Djava.class.path=" "@datadir@/@PACKAGE@/tools.zip";
    }

  /* Terminate vm_args.options with a NULL element. */
  vm_args.options = (JavaVMOption*) realloc (vm_args.options, (vm_args.nOptions + 1) * sizeof (JavaVMOption));
  if (vm_args.options == NULL)
    {
      g_printerr ("appletviewer: realloc failed.\n");
      goto destroy;
    }
  vm_args.options[vm_args.nOptions].optionString = NULL;

  /* Terminate non_vm_argv with a NULL element. */
  non_vm_argv = (char**) realloc (non_vm_argv, (non_vm_argc + 1) * sizeof (char*));
  if (non_vm_argv == NULL)
    {
      g_printerr ("appletviewer: realloc failed.\n");
      goto destroy;
    }
  non_vm_argv[non_vm_argc] = NULL;

  vm_args.version = JNI_VERSION_1_2;
  vm_args.ignoreUnrecognized = JNI_TRUE;

  result = JNI_CreateJavaVM (&jvm, &tmp.void_env, &vm_args);

  if (result < 0)
    {
      g_printerr ("appletviewer: couldn't create virtual machine\n");
      goto destroy;
    }

  env = tmp.jni_env;

  string_class_id = (*env)->FindClass (env, "java/lang/String");
  if (string_class_id == NULL)
    {
      g_printerr ("appletviewer: FindClass failed.\n");
      goto destroy;
    }

  args_array = (*env)->NewObjectArray (env, non_vm_argc, string_class_id, NULL);
  if (args_array == NULL)
    {
      g_printerr ("appletviewer: NewObjectArray failed.\n");
      goto destroy;
    }

  for (i = 0; i < non_vm_argc; i++)
    {
      str = (*env)->NewStringUTF (env, non_vm_argv[i]);
      if (str == NULL)
	{
	  g_printerr ("appletviewer: NewStringUTF failed.\n");
	  goto destroy;
	}

      (*env)->SetObjectArrayElement (env, args_array, i, str);
    }

  class_id = (*env)->FindClass (env, "gnu/classpath/tools/appletviewer/Main");
  if (class_id == NULL)
    {
      g_printerr ("appletviewer: FindClass failed.\n");
      goto destroy;
    }

  method_id = (*env)->GetStaticMethodID (env, class_id, "main", "([Ljava/lang/String;)V");

  if (method_id == NULL)
    {
      g_printerr ("appletviewer: GetStaticMethodID failed.\n");
      goto destroy;
    }

  (*env)->CallStaticVoidMethod (env, class_id, method_id, args_array);

 destroy:

  if (env != NULL)
    {
      if ((*env)->ExceptionOccurred (env))
	(*env)->ExceptionDescribe (env);

      if (jvm != NULL)
	(*jvm)->DestroyJavaVM (jvm);
    }

  return 1;
}