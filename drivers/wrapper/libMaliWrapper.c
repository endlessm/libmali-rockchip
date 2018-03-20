#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#ifdef DEBUG
#define DEBUG_MSG(format,...) fprintf(stderr, "%s %d:"format"\n", __func__, __LINE__, __VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif
/* 
 * The function below emulates the behaivor of the folowing xlib patch 
 * https://patchwork.freedesktop.org/patch/64783/mbox/
 *
 * Compilation and link command:
 *
 * gcc -c -fPIC -O2 libMaliWrapper.c
 * ld -Bshareable -o libMaliWrapper.so libMaliWrapper.o -L. -lMali
 *
 */

typedef struct {
  Display *dpy;
  XErrorHandler handler;
} HandlerData;

static pthread_mutex_t theMutex = PTHREAD_MUTEX_INITIALIZER;
static HandlerData *handlers = NULL;
static int n_handlers = 0;
static XErrorHandler default_handler = NULL;

static HandlerData *
handlers_lookup (Display *dpy)
{
  int i;

  for (i = 0; i < n_handlers; i++)
    {
      if (handlers[i].dpy == dpy)
	return &handlers[i];
    }

  return NULL;
}

static int 
magic_error_handler (Display *dpy, XErrorEvent *event)
{
  HandlerData *data;

  DEBUG_MSG ("dpy=%p event=%p", dpy, event);

  /* Lookup for specific display error handler */
  if ((data = handlers_lookup (dpy)) && data->handler)
    return (*data->handler) (dpy, event);
  else if (default_handler)
    return (*default_handler) (dpy, event);
  else
    return 0;
}

XErrorHandler
XDisplaySetErrorHandler (Display *dpy, XErrorHandler handler)
{
  static int initialized = 0;
  XErrorHandler old_handler = NULL;
  HandlerData *data;

  pthread_mutex_lock (&theMutex);

  DEBUG_MSG ("dpy=%p handler=%p", dpy, handler);

  if (!initialized)
    {
      default_handler = XSetErrorHandler (magic_error_handler);
      initialized = 1;
      DEBUG_MSG ("default_handler=%p", default_handler);
    }

  /* Lookup old handler */
  if ((data = handlers_lookup (dpy)))
    {
      old_handler = data->handler;
      data->handler = handler;
      DEBUG_MSG ("\told_handler=%p", old_handler);
    }
  else
    {
      /* Insert new handler */
      n_handlers++;

      if (handlers)
        handlers = realloc (handlers, n_handlers * sizeof (HandlerData));
      else
        handlers = calloc (n_handlers, sizeof (HandlerData));

      handlers[n_handlers-1].dpy = dpy;
      handlers[n_handlers-1].handler = handler;
      DEBUG_MSG ("\tn_handlers=%d", n_handlers);
    }

  pthread_mutex_unlock (&theMutex);

  return old_handler;
}


