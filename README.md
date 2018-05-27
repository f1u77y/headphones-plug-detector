# Headphones plug detector

## What does it do

This program uses [jacklistener](https://github.com/gentoo-root/jacklistener) to detect
when you plug out your headphones and pauses music for you.
When you plug them in again, it resumes all the players that were paused by this
program.

You need jacklistener and MPRIS-compliant audio players for
this program to work. If you're using some common DE (GNOME, KDE, Unity) and see
the player widget on panel, then your player is MPRIS-compliant. Almost all common
players with GUI (Clementine, Amarok, Deadbeef, etc.) are compliant (but some of them
need enabling built-in plugin for this). If you're using MPD, you could run some proxy
client like mpDris2 or use GUI client that has it out of the box like Cantata.

If you're using some non-common DE/WM or music player, you probably know what are you
doing :)

## Building

    mkdir build
    cd build
    cmake ..
    make

## Dependencies

- `jacklistener`
- `glib2`
