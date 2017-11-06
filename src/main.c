#include <pulse/subscribe.h>
#include <pulse/mainloop.h>
#include <pulse/introspect.h>
#include <glib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "mpris.h"


enum port_type {
  PORT_TYPE_UNKNOWN,
  PORT_TYPE_SPEAKER,
  PORT_TYPE_HEADPHONES,
};

enum port_type last_port_type = PORT_TYPE_UNKNOWN;

enum port_type
get_port_type(pa_sink_port_info* port)
{
  if (!port) {
    return PORT_TYPE_UNKNOWN;
  } else if (strstr(port->name, "speaker")) {
    return PORT_TYPE_SPEAKER;
  } else if (strstr(port->name, "headphones")) {
    return PORT_TYPE_HEADPHONES;
  } else {
    return PORT_TYPE_UNKNOWN;
  }
}

void
sink_changed_callback(pa_context* G_GNUC_UNUSED ctx,
                      const pa_sink_info* info,
                      int eol,
                      void* userdata)
{
  if (eol) return;

  GList** last_paused_players = userdata;

  enum port_type cur_port_type = get_port_type(info->active_port);
  if (cur_port_type == last_port_type) {
    return;
  } else if (cur_port_type == PORT_TYPE_SPEAKER) {
    GList* players = get_playing_players();
    for (GList* elem = players; elem != NULL; elem = elem->next) {
      pause_player_by_name(elem->data);
      *last_paused_players = g_list_prepend(*last_paused_players, strdup(elem->data));
    }
  } else if (cur_port_type == PORT_TYPE_HEADPHONES) {
    GList* elem = *last_paused_players;
    while (elem != NULL) {
      GList* next = elem->next;
      play_player_by_name(elem->data);
      *last_paused_players = g_list_delete_link(*last_paused_players, elem);
      elem = next;
    }
  }
  last_port_type = cur_port_type;
}

void
subscribe_callback(pa_context* ctx,
                   pa_subscription_event_type_t type,
                   uint32_t idx,
                   void* userdata)
{
  if ((type & PA_SUBSCRIPTION_EVENT_TYPE_MASK) != PA_SUBSCRIPTION_EVENT_CHANGE) return;
  pa_context_get_sink_info_by_index(ctx, idx, sink_changed_callback, userdata);
}


void
context_state_callback(pa_context* ctx, void* userdata)
{
  switch (pa_context_get_state(ctx)) {
  case PA_CONTEXT_READY:
    pa_context_set_subscribe_callback(ctx, subscribe_callback, userdata);
    if (!pa_context_subscribe(ctx, (pa_subscription_mask_t)
                              PA_SUBSCRIPTION_MASK_SINK,
                              NULL, userdata))
    {
      perror("Subscribtion failed!\n");
      return;
    }
  default:
    return;
  }
}

int
main()
{
  pa_mainloop* loop = pa_mainloop_new();
  pa_mainloop_api* api = pa_mainloop_get_api(loop);

  pa_proplist* proplist = pa_proplist_new();
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "HPD");
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "me.f1u77y.hpd");
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "0.1.0");

  pa_context* ctx = pa_context_new_with_proplist(api, NULL, proplist);
  assert(ctx);

  pa_proplist_free(proplist);
  proplist = NULL;

  GList* players = NULL;

  pa_context_set_state_callback(ctx, context_state_callback, &players);

  if (pa_context_connect(ctx, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
    perror("Connection failed\n");
    exit(1);
  }

  int retval = 0;
  pa_mainloop_run(loop, &retval);
  return retval;
}
