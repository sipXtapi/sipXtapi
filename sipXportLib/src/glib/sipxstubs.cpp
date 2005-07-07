/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

/*
 * Some of the code in this file was derived or extracted from
 * other glib files to minimize or elimate the dependency on 
 * the rest of the glib code.
 */

//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <utl/UtlDefs.h>
#include <glib/glib.h>

#ifdef G_OS_WIN32
#define STRICT
#include <windows.h>
#endif /* G_OS_WIN32 */

#define NOT_IMPLEMENTED(NAME) osPrintf("%s not implemented: %s", NAME, __FILE__);

// FUNCTIONS
static void g_thread_fail (void);

// STATIC VARIABLE INITIALIZATIONS
// from gthread.c
GThreadFunctions g_thread_functions_for_glib_use = {
  (GMutex*(*)())g_thread_fail,                 /* mutex_new */
  NULL,                                        /* mutex_lock */
  NULL,                                        /* mutex_trylock */
  NULL,                                        /* mutex_unlock */
  NULL,                                        /* mutex_free */
  (GCond*(*)())g_thread_fail,                  /* cond_new */
  NULL,                                        /* cond_signal */
  NULL,                                        /* cond_broadcast */
  NULL,                                        /* cond_wait */
  NULL,                                        /* cond_timed_wait  */
  NULL,                                        /* cond_free */
  (GPrivate*(*)(GDestroyNotify))g_thread_fail, /* private_new */
  NULL,                                        /* private_get */
  NULL,                                        /* private_set */
  (void(*)(GThreadFunc, gpointer, gulong, 
	   gboolean, gboolean, GThreadPriority, 
	   gpointer, GError**))g_thread_fail,  /* thread_create */
  NULL,                                        /* thread_yield */
  NULL,                                        /* thread_join */
  NULL,                                        /* thread_exit */
  NULL,                                        /* thread_set_priority */
  NULL                                         /* thread_self */
}; 

// State to prevent multiple stub error messages
// for g_thread_init
UtlBoolean sipxstub_g_thread_init_called = FALSE;

// From gthread.c
GLIB_VAR gboolean               g_threads_got_initialized;
gboolean g_threads_got_initialized = FALSE;

/* ============================ FUNCTIONS ================================= */

/*
The following are stubs to mimimize the amount of code
that is needed from the glib project.  Currently memory
pooling is turned off (DISABLE_MEM_POOLS) to prevent
the need for the locking utilities in the g_threads
library.  Some of these stubs may no longer be needed
a second round of optimization (i.e. reducing) of these
stubs and the number of glib header files can probably
eliminate some more code.
*/


extern "C" void
g_log (const gchar   *log_domain,
       GLogLevelFlags log_level,
       const gchar   *format,
       ...)
{
    NOT_IMPLEMENTED("g_log");
}


extern "C" void
g_logv (const gchar   *log_domain,
	GLogLevelFlags log_level,
	const gchar   *format,
	va_list	       args1)
{
    NOT_IMPLEMENTED("g_logv");
}

// from gutils.c
extern "C" void
g_print (const gchar *format,
	 ...)
{
    NOT_IMPLEMENTED("g_print");
}

extern "C" guint
g_direct_hash (gconstpointer v)
{
  return GPOINTER_TO_UINT (v);
}


// from gstrfuncs.c
extern "C" gchar*
g_strdup (const gchar *str)
{
  gchar *new_str;
  gsize length;

  if (str)
    {
      length = strlen (str) + 1;
      new_str = g_new (char, length);
      memcpy (new_str, str, length);
    }
  else
    new_str = NULL;

  return new_str;
}

// get a new mutex??
extern "C" GMutex *
g_static_mutex_get_mutex_impl (GMutex** mutex)
{
    NOT_IMPLEMENTED("g_static_mutex_get_mutex_impl");
    return NULL;
}

//from gthread.c
static void
g_thread_fail (void)
{
  g_error ("The thread system is not yet initialized.");
}


extern "C" void
g_thread_init (GThreadFunctions* init)
{
    // Only print the message the first time this is called
    if(!sipxstub_g_thread_init_called)
    {
        NOT_IMPLEMENTED("g_thread_init");
        sipxstub_g_thread_init_called = TRUE;
    }
}

// From gmain.c
/**
 * g_get_current_time:
 * @result: #GTimeVal structure in which to store current time.
 * 
 * Equivalent to the UNIX gettimeofday() function, but portable.
 **/
void
g_get_current_time (GTimeVal *result)
{
#ifndef G_OS_WIN32
  struct timeval r;

  g_return_if_fail (result != NULL);

  /*this is required on alpha, there the timeval structs are int's
    not longs and a cast only would fail horribly*/
  gettimeofday (&r, NULL);
  result->tv_sec = r.tv_sec;
  result->tv_usec = r.tv_usec;
#else
  /* Avoid calling time() except for the first time.
   * GetTickCount() should be pretty fast and low-level?
   * I could also use ftime() but it seems unnecessarily overheady.
   */
  static DWORD start_tick = 0;
  static time_t start_time;
  DWORD tick;

  g_return_if_fail (result != NULL);
 
  if (start_tick == 0)
    {
      start_tick = GetTickCount ();
      time (&start_time);
    }

  tick = GetTickCount ();

  result->tv_sec = (tick - start_tick) / 1000 + start_time;
  result->tv_usec = ((tick - start_tick) % 1000) * 1000;
#endif
}


