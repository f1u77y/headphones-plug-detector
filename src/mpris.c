#include "mpris.h"

#include "mpris-core.h"
#include "mpris-player.h"
#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include <stdio.h>

static GDBusProxy*
org_freedesktop_dbus_proxy(GError** error)
{
  GDBusConnection* bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, error);
  if (!bus) {
    return NULL;
  }

  GDBusProxy* proxy = g_dbus_proxy_new_sync(bus,
                                            G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                            NULL, // interface_info
                                            "org.freedesktop.DBus",
                                            "/org/freedesktop/DBus",
                                            "org.freedesktop.DBus",
                                            NULL, // cancellable
                                            error);
  g_object_unref(bus);
  return proxy;
}

static GList*
get_all_players()
{
  GError* error = NULL;

  GDBusProxy* proxy = org_freedesktop_dbus_proxy(&error);
  if (!proxy) {
    g_critical("%s", error->message);
    g_error_free(error);
    return NULL;
  }

  GVariant* response = g_dbus_proxy_call_sync(proxy,
                                              "ListNames",
                                              g_variant_new("()"),
                                              G_DBUS_CALL_FLAGS_NONE,
                                              -1,
                                              NULL,
                                              &error);
  if (!response) {
    g_critical("%s", error->message);
    g_error_free(error);
    g_object_unref(proxy);
    return NULL;
  }

  GList* result = NULL;

  GVariantIter* iter;
  gchar* name;
  g_variant_get(response, "(as)", &iter);
  while (g_variant_iter_loop(iter, "s", &name)) {
    if (strstr(name, "org.mpris.MediaPlayer2") == name) {
      result = g_list_prepend(result, strdup(name));
    }
  }

  result = g_list_reverse(result);
  return result;
}

static MediaPlayer2Player*
get_player_by_name(const gchar* name, GError** error)
{
  GDBusConnection* bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, error);
  if (!bus) {
    return NULL;
  }
  MediaPlayer2Player* player =
      media_player2_player_proxy_new_sync(bus,
                                          G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                          name,
                                          "/org/mpris/MediaPlayer2",
                                          NULL, // cancellable
                                          error);
  g_object_unref(bus);
  return player;
}

static gboolean
is_player_playing(const gchar* name)
{
  MediaPlayer2Player* player = get_player_by_name(name, NULL);
  if (!player) {
    g_object_unref(player);
    return FALSE;
  }

  const gchar* status = media_player2_player_get_playback_status(player);
  gboolean is_playing = strcmp(status, "Playing") == 0;
  g_object_unref(player);
  return is_playing;
}

GList*
get_playing_players()
{
  GList* players = get_all_players();
  GList* elem = players;
  while (elem != NULL) {
    GList* next = elem->next;
    if (!is_player_playing(elem->data)) {
      players = g_list_delete_link(players, elem);
    }
    elem = next;
  }
  return players;
}

typedef gboolean (*WithPlayerCallback)(MediaPlayer2Player*, GCancellable*, GError**);

static void
with_player_do(const gchar* name, WithPlayerCallback callback)
{
  GError* error = NULL;
  MediaPlayer2Player* player = get_player_by_name(name, &error);
  if (!player) {
    g_warning("%s", error->message);
    g_error_free(error);
    return;
  }

  gboolean result = callback(player, NULL, &error);
  g_object_unref(player);
  if (!result) {
    g_warning("%s", error->message);
    g_error_free(error);
  }
}

void
pause_player_by_name(const gchar* name)
{
  return with_player_do(name, media_player2_player_call_pause_sync);
}

void
play_player_by_name(const gchar* name)
{
  return with_player_do(name, media_player2_player_call_play_sync);
}
