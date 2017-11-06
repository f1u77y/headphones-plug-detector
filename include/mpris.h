#pragma once

#include <glib.h>

GList*
get_playing_players();

void
watch_mpris_names();

void
pause_player_by_name(const gchar*);

void
play_player_by_name(const gchar*);
