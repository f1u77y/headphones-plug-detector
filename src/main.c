#include <glib.h>
#include <gio/gio.h>

#include "mpris.h"

void
pause_all_players(GList** last_paused_players)
{
  GList* players = get_playing_players();
  for (GList* elem = players; elem != NULL; elem = elem->next) {
    pause_player_by_name(elem->data);
    *last_paused_players = g_list_prepend(*last_paused_players, strdup(elem->data));
  }
}

void
resume_all_players(GList** last_paused_players)
{
  GList* elem = *last_paused_players;
  while (elem != NULL) {
    GList* next = elem->next;
    play_player_by_name(elem->data);
    *last_paused_players = g_list_delete_link(*last_paused_players, elem);
    elem = next;
  }
}

void
on_jacklistener_signal(GDBusProxy* proxy,
                       gchar* sender_name,
                       gchar* signal_name,
                       GVariant* parameters,
                       gpointer user_data)
{
  if (!g_strcmp0(signal_name, "removed")) {
    pause_all_players(user_data);
  } else if (!g_strcmp0(signal_name, "inserted")) {
    resume_all_players(user_data);
  }
}

gboolean
watch_jacklistener(GList** players)
{
  GError *error = NULL;
  GDBusConnection *bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

  if (!bus) {
    g_critical("%s", error->message);
    g_error_free(error);
    return FALSE;
  }

  GDBusProxy* headphone = g_dbus_proxy_new_sync(bus,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL, // interface info, XXX
                                                "org.ude.jacklistener",
                                                "/jack/headphone",
                                                "org.ude.jacklistener",
                                                NULL, // cancellable
                                                &error);
  if (!headphone) {
    g_critical("%s", error->message);
    g_error_free(error);
    return FALSE;
  }

  g_signal_connect(headphone, "g-signal", G_CALLBACK(on_jacklistener_signal), players);
}

int
main()
{
  GList* players = NULL;
  GMainLoop* loop = g_main_loop_new(NULL, FALSE);
  watch_jacklistener(&players);
  g_main_loop_run(loop);
  return 0;
}
