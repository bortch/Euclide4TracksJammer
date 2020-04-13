#pragma once

#ifndef __E4T_CONSTANT_HPP__
#define __E4T_CONSTANT_HPP__

#include <RK002.h>

/*
 * App info 
 */
#define APP_NAME "Euclide4TracksJammer"
#define APP_AUTHOR "Soultracker"
#define APP_VERSION "0.1.9"
#define APP_GUID "4908c586-6b7d-492c-9368-6eb81bf6b0e4"

/**
 * Debug 
 */
#define DEBUG 0

#define LOG(fmt, ...)                 \
  do                                  \
  {                                   \
    if (DEBUG)                        \
      RK002_printf(fmt, __VA_ARGS__); \
  } while (0)

/*
* MIDI
*/
#define RESOLUTION 24

//Tracks - param
#define MAX_TRACKS 4
#define MAX_BUFFER_SIZE 16
#define MAX_STEP 16
#define MAX_ELEMENT_SIZE 16 // polyphony

#define ID_TRACKS_0 0
#define ID_TRACKS_1 1
#define ID_TRACKS_2 2
#define ID_TRACKS_3 3

#define MODE_OFFSET 1
#define MODE_DURATION 2
#define MODE_ARP_CHORD 3
#define MODE_EUCLIDE 4
#define MODE_PATTERN 5
#define MODE_BPM 6

// User interface (A0->B0)
#define KEY_BASE_DEFAULT 21
#define KEY_DEC 0
#define KEY_CLEAR_RESET 1
#define KEY_INC 2
#define KEY_NOTE_OFFSET_MODE 3
#define KEY_NOTE_DURATION_MODE 4
#define KEY_TRACK_0 5
#define KEY_ARP_CHORD_MODE 6
#define KEY_TRACK_1 7
#define KEY_TRACK_2 8
#define KEY_EUCLIDE_MODE 9
#define KEY_TRACK_3 10
#define KEY_PATTERN_MODE 11
#define KEY_OPTION_1 12
#define KEY_BPM_MODE 13
#define KEY_OPTION_10 14

// UI - param
#define INC_VALUE 1
#define DEC_VALUE -1
#define OPTION_PARAM_10 2
// Mode Chord / Arp
#define MODE_CHORDS true
#define MODE_ARP false
// Note Duration - param
#define DURATION_MODE 5
// Note Duration - generation type
#define DURATION_PARAM_ONE 0 // 1 step
#define DURATION_PARAM_RANDOM_ONCE 1 // Random once
#define DURATION_PARAM_RANDOM_EACH 2 // Random Each
#define DURATION_PARAM_EUCLIDE 3 // NumberOfStep/NumberOfHit
#define DURATION_PARAM_HALF_EUCLIDE 4 // NumberOfStep/(NumberOfHit*2)

#define EMPTY 0

#endif
