#include <pulse/subscribe.h>
#include <pulse/mainloop.h>
#include <pulse/introspect.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>


#define UNUSED __attribute__((unused))


void
sink_changed_callback(pa_context * UNUSED ctx,
                      const pa_sink_info *info,
                      int eol,
                      void * UNUSED userdata)
{
  if (eol) return;

  pa_sink_port_info *port = info->active_port;
  if (!port) {
    perror("port is NULL\n");
    return;
  }
  printf("new port name is '%s'\n", port->name);
}

void
subscribe_callback(pa_context *ctx,
                   pa_subscription_event_type_t type,
                   uint32_t idx,
                   void * UNUSED userdata)
{
  if ((type & PA_SUBSCRIPTION_EVENT_TYPE_MASK) != PA_SUBSCRIPTION_EVENT_CHANGE) return;

  printf("Sink %" PRIu32 " changed\n", idx);
  pa_context_get_sink_info_by_index(ctx, idx, sink_changed_callback, NULL);
}



void
context_state_callback(pa_context *ctx, void * UNUSED userdata) {
  switch (pa_context_get_state(ctx)) {
  case PA_CONTEXT_READY:
    pa_context_set_subscribe_callback(ctx, subscribe_callback, NULL);
    if (!pa_context_subscribe(ctx, (pa_subscription_mask_t)
                              PA_SUBSCRIPTION_MASK_SINK,
                              NULL, NULL))
    {
      perror("Subscribtion failed!\n");
      return;
    }
  default:
    return;
  }
}


int
main() {
  pa_mainloop *loop = pa_mainloop_new();
  pa_mainloop_api *api = pa_mainloop_get_api(loop);

  pa_proplist *proplist = pa_proplist_new();
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "HPD");
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "me.f1u77y.hpd");
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "0.1.0");

  pa_context *ctx = pa_context_new_with_proplist(api, NULL, proplist);
  assert(ctx);

  pa_proplist_free(proplist);
  proplist = NULL;

  pa_context_set_state_callback(ctx, context_state_callback, NULL);

  if (pa_context_connect(ctx, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
    perror("Connection failed\n");
    exit(1);
  }

  int retval = 0;
  pa_mainloop_run(loop, &retval);
  return retval;
}
