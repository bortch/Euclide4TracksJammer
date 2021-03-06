#include <Arduino.h>
#include <RK002.h>
#include "Track.hpp"
#include "NoteOffHandler.hpp"
#include "Constant.hpp"
#include "State.hpp"

RK002_DECLARE_INFO(APP_NAME, APP_AUTHOR, APP_VERSION, APP_GUID)

/**
 * Each track contains max 16 notes.
 * Each track can have a different length (16 steps by default).
 * An offset can be added to each track.
 * Each note is triggered according to a Euclidean rhythm defined by track (by defining the number of trigs)
 * Each track can be played in arpeggio mode (one note per trig) or in chord mode (all notes are played at each trig)
 * The duration of the notes can be generated by following one of the 5 choices [Same duration of a step, Fixed Random, Live Random, NumberOfStep / NumberOfHit, NumberOfStep / (NumberOfHit * 2)]
 * For now, it only uses an internal clock.
 * The BPM is 120 by default, could be increased / decreased by 1 or 10.
 * It uses midi channel 1.
 * 
 * User interface:
 * A 0 : [-] decrease key
 * A#0 : Clear/Reset
 * B 0 : [+] increase key
 * C 1 : Note/Offset Mode
 * C#1 : Note Duration Mode
 * D 1 : Track 1 selection 
 * D#1 : Arpeggio/chords Mode 
 * E 1 : Track 2 selection
 * F 1 : Track 3 selection
 * F#1 : Euclide Trig Mode
 * G 1 : Track 4 selection
 * G#1 : Pattern Length Mode
 * A 1 : Factor 1 option
 * A#1 : BPM Mode
 * B 1 : Factor 10 option
 * 
 * The general operation is as follows: 
 * select a track, then a mode and modify (increase or decrease) its value.
 * 
 * Example of key combination:
 *  - Delete the notes of a track:
 *    ([D1] or [E1] or [F1] or [G1]) + [C1] + [A # 0]
 *  - Add an offset in a track:
 *    ([D1] or [E1] or [F1] or [G1]) + [C1] + [B0]
 *  - Change the generation mode of the note duration:
 *    ([D1] or [E1] or [F1] or [G1]) + [C # 1] + [A # 0]
 * 
 * Example of use:
 *  1. Press and hold D1 to select Track 1
 *  2. While holding D1, enter few notes, they are now recorded in track 1
 *  3. Then press and hold D1 and F#1 (to enter Euclide Mode for the Track 1) 
 *  4. While holding those keys, set the number of hit for the pattern by pressing B0.
 *  5. The sequence will begin to play the entered notes when you let go. 
 * 
 *  Github: https://github.com/bortch/Euclide4TracksJammer
 *  
 * */

// RK002_DECLARE_PARAM(name,flags,min,max,def)
RK002_DECLARE_PARAM(MIDICHN, 1, 0, 15, 0)      // defines the operating MIDI channel
RK002_DECLARE_PARAM(BPM, 1, 30, 240, 120);     // defines the base BPM
RK002_DECLARE_PARAM(ROOT_NOTE, 1, 21, 74, 21); //A0 by default

static byte tick = 0;
static byte clockStep = 0;

static NoteOffHandler noteOffHandler = NoteOffHandler();
static Track *tracks[MAX_TRACKS];
static State state = State();

bool RK002_onNoteOn(byte channel, byte key, byte velocity)
{
  byte command = key - RK002_paramGet(ROOT_NOTE);
  LOG("Note ON (channel=%d, key=%d, velocity=%d) command: %d", channel, key, velocity, command);
  bool thru = false;
  switch (command)
  {
  // mode
  case KEY_NOTE_OFFSET_MODE:
    bitSet(state.mode, MODE_OFFSET);
    break;
  case KEY_NOTE_DURATION_MODE:
    bitSet(state.mode, MODE_DURATION);
    break;
  case KEY_ARP_CHORD_MODE:
    //bitSet(state.mode, MODE_ARP_CHORD);
    forSelectedTrackDo(&switchArpChordsMode);
    break;
  case KEY_EUCLIDE_MODE:
    bitSet(state.mode, MODE_EUCLIDE);
    break;
  case KEY_PATTERN_MODE:
    bitSet(state.mode, MODE_PATTERN);
    break;
  case KEY_BPM_MODE:
    bitSet(state.mode, MODE_BPM);
    break;
    // tracks
  case KEY_TRACK_0:
    bitSet(state.track, ID_TRACKS_0);
    break;
  case KEY_TRACK_1:
    bitSet(state.track, ID_TRACKS_1);
    break;
  case KEY_TRACK_2:
    bitSet(state.track, ID_TRACKS_2);
    break;
  case KEY_TRACK_3:
    bitSet(state.track, ID_TRACKS_3);
    break;
  // operate
  case KEY_CLEAR_RESET:
    if (state.isModeActive(MODE_EUCLIDE))
    {
      forSelectedTrackDo(&euclideClear);
    }
    if (state.isModeActive(MODE_PATTERN))
    {
      changeTrackLength(MAX_STEP);
    }
    if (state.isModeActive(MODE_OFFSET))
    {
      forSelectedTrackDo(&clearTrack);
    }
    if (state.isModeActive(MODE_DURATION))
    {
      forSelectedTrackDo(&updateNotesDuration);
    }
    if (state.isModeActive(MODE_BPM))
    {
      RK002_paramSet(BPM, 120);
      RK002_clockSetTempo(RK002_paramGet(BPM));
    }
    break;

  case KEY_INC:
    if (state.isModeActive(MODE_EUCLIDE))
    {
      forSelectedTrackDo(&euclideSetTrigs);
    }
    if (state.isModeActive(MODE_PATTERN))
    {
      changeTrackLength(INC_VALUE);
    }
    if (state.isModeActive(MODE_OFFSET))
    {
      forSelectedTrackDo(&moveTrigsLeft);
    }
    if (state.isModeActive(MODE_DURATION))
    {
      forSelectedTrackDo(&changeNotesDuration);
    }
    if (state.isModeActive(MODE_BPM))
    {
      RK002_paramSet(BPM, constrain(RK002_paramGet(BPM) + INC_VALUE * state.optionFactor(), 30, 240));
    }
    break;

  case KEY_DEC:
    if (state.isModeActive(MODE_EUCLIDE))
    {
      //reduce euclide hits
    }
    if (state.isModeActive(MODE_PATTERN))
    {
      changeTrackLength(DEC_VALUE);
    }
    if (state.isModeActive(MODE_OFFSET))
    {
      forSelectedTrackDo(&moveTrigsRight);
    }
    if (state.isModeActive(MODE_DURATION))
    {
      forSelectedTrackDo(&changeNotesDuration);
    }
    if (state.isModeActive(MODE_BPM))
    {
      RK002_paramSet(BPM, constrain(RK002_paramGet(BPM) + DEC_VALUE * state.optionFactor(), 30, 240));
    }
    break;
  // option
  case KEY_OPTION_10:
    // modify by x*10
    bitSet(state.bpm, OPTION_PARAM_10);
    break;
  case KEY_OPTION_1:
    // modify by x*1
    bitClear(state.bpm, OPTION_PARAM_10);
    break;
  default:
    recordKey(key, velocity);
    thru = true;
    break;
  }
  return thru;
}

//
// handle note off
//
bool RK002_onNoteOff(byte channel, byte key, byte velocity)
{
  byte command = key - RK002_paramGet(ROOT_NOTE);
  LOG("Note OFF (channel=%d, key=%d, velocity=%d) command: %d", channel, key, velocity, command);
  bool thru = false;
  switch (command)
  {
  // mode
  case KEY_NOTE_OFFSET_MODE:
    bitClear(state.mode, MODE_OFFSET);
    break;
  case KEY_NOTE_DURATION_MODE:
    bitClear(state.mode, MODE_DURATION);
    break;
  /* 
  case KEY_ARP_CHORD_MODE:
    bitClear(state.mode, MODE_ARP_CHORD);
    break;*/
  case KEY_EUCLIDE_MODE:
    bitClear(state.mode, MODE_EUCLIDE);
    break;
  case KEY_PATTERN_MODE:
    bitClear(state.mode, MODE_PATTERN);
    break;
  case KEY_BPM_MODE:
    bitClear(state.mode, MODE_BPM);
    break;
    // tracks
  case KEY_TRACK_0:
    // unset armedTrack bit for id track
    bitClear(state.track, ID_TRACKS_0);
    // recompute euclidean rythm if new trigs has been added
    forTrigsChangedTrackDo(&recomputeEuclide);
    break;
  case KEY_TRACK_1:
    bitClear(state.track, ID_TRACKS_1);
    forTrigsChangedTrackDo(&recomputeEuclide);
    break;
  case KEY_TRACK_2:
    bitClear(state.track, ID_TRACKS_2);
    forTrigsChangedTrackDo(&recomputeEuclide);
    break;
  case KEY_TRACK_3:
    bitClear(state.track, ID_TRACKS_3);
    forTrigsChangedTrackDo(&recomputeEuclide);
    break;
    // option
  case KEY_OPTION_10:
    // modify by x*100
    bitClear(state.bpm, OPTION_PARAM_10);
    break;
  case KEY_OPTION_1:
    // modify by x*10
    bitClear(state.bpm, OPTION_PARAM_10);
    break;
  default:
    thru = true;
    break;
  }
  // block original key
  return thru;
}

bool RK002_onClock()
{
  tick = (tick + 1) % RESOLUTION;
  // play
  //LOG("Clock message (tick=%d)",tick);
  if (tick == 0)
  {
    clockStep = (clockStep + 1) % MAX_STEP;
    LOG("Clock step %d", clockStep);
    // for each step of each track,
    // kill notes that needs to be killed.
    noteOffHandler.flushNote(clockStep, RK002_paramGet(MIDICHN));

    byte trackIndex = MAX_TRACKS;
    do
    {
      sendNotesOn(trackIndex - 1, clockStep);
    } while (--trackIndex);
  }
  // prepare next step
  return false;
}

void sendNotesOn(const byte trackIndex, const byte clockStep)
{
  if (tracks[trackIndex]->isHit(clockStep))
  {
    // play note/chords @ this step
    LOG("tracks[%d].sendNotesOn(%d)", trackIndex, clockStep);
    tracks[trackIndex]->sendNotesOn(clockStep);
  }
}

void RK002_onParamChange(unsigned param_nr, int val)
{
  switch (param_nr)
  {
  case BPM:
    RK002_clockSetTempo(RK002_paramGet(BPM) * 10);
    break;
  }
}

void euclide(const byte trigs, const int trackNumber)
{
  LOG("euclide trigs:%d in tracks[%d]", trigs, trackNumber);
  if (trigs > 0)
  {
    byte cur = MAX_STEP;
    for (byte i = 0; i < MAX_STEP; i++)
    {
      if (cur >= MAX_STEP)
      {
        cur -= MAX_STEP;
        tracks[trackNumber]->setHit(i);
      }
      else
      {
        tracks[trackNumber]->setRest(i);
      }
      cur += trigs;
    }
  }
  else
  {
    // avoid computation
    // and clear all trigs
    tracks[trackNumber]->clearTrigs();
  }
}

/**
 * Recompute Euclidean Pattern if trigsChangesExists
 */
void recomputeEuclide(const byte trackIndex)
{
  LOG("recomputeEuclide in tracks[%d]", trackIndex);
  // recompute rythm
  euclide(tracks[trackIndex]->getTrigs(), trackIndex);
  // reset trig's flag
  bitClear(state.trig, tracks[trackIndex]->trackID());
}

/**
 * add new trig to selected tracks for selected tracks
 */
void euclideSetTrigs(const byte trackIndex)
{
  LOG("euclideSetTrigs in tracks[%d]", trackIndex);
  // add trigs
  tracks[trackIndex]->increaseTrigs(1);
  // recomputing Euclidean rythm on Track selection Key's Note Off
  // set the changedTrigs of the corresponding flag
  bitSet(state.trig, tracks[trackIndex]->trackID());
}

void forSelectedTrackDo(void (*function)(const byte))
{
  if (state.track > 0)
  {
    byte trackIndex = MAX_TRACKS - 1;
    do
    {
      if (state.isTrackSelected(tracks[trackIndex]->trackID()))
      {
        function(trackIndex);
      }
    } while (trackIndex--);
  }
}

void forTrigsChangedTrackDo(void (*function)(const byte))
{
  if (state.trigsChangesExists())
  {
    byte trackIndex = MAX_TRACKS - 1;
    do
    {
      if (state.trigsChanged(tracks[trackIndex]->trackID()))
      {
        function(trackIndex);
      }
    } while (trackIndex--);
  }
}

/**
 * Clear pattern of selected tracks
 */
void euclideClear(const byte trackIndex)
{
  LOG("euclideClear in tracks[%d]", trackIndex);
  // clear all trigs
  tracks[trackIndex]->clearTrigs();
  // reset trig's flag
  // set the changedTrigs of the corresponding flag
  bitSet(state.trig, tracks[trackIndex]->trackID());
  // recomputing Euclidean rythm on Track selection Key's Note Off
}

/*
* Move trig one position to the left for selected track 
*/
void moveTrigsLeft(const byte trackIndex)
{
  LOG("moveTrigsLeft in tracks[%d]", trackIndex);
  tracks[trackIndex]->decreaseOffset(1);
}

/*
* Move trig one position to the right for selected tracks 
*/
void moveTrigsRight(const byte trackIndex)
{
  LOG("moveTrigsRight in tracks[%d]", trackIndex);
  tracks[trackIndex]->increaseOffset(1);
}

/*
* Records incomming keys and velocity to armed tracks
*/
void recordKey(const byte key, const byte velocity)
{
  if (state.track > 0)
  {
    byte trackIndex = MAX_TRACKS - 1;
    do
    {
      if (state.isTrackSelected(tracks[trackIndex]->trackID()))
      {
        LOG("recordKey(%d) in tracks[%d]", key, trackIndex);
        tracks[trackIndex]->addNoteOn(key, velocity);
      }
    } while (trackIndex--);
  }
}

/** 
 * Clear selected Tracks for selected tracks
 */
void clearTrack(const byte trackIndex)
{
  LOG("clear Track[%d]", trackIndex);
  tracks[trackIndex]->clearNotes();
}

/**
 * Switch between ARP or CHORD modifyedTrigsExists for selected tracks
 */
void switchArpChordsMode(const byte trackIndex)
{
  LOG("Switch Mode of tracks[%d]", trackIndex);
  tracks[trackIndex]->switchArpChordsMode();
}

/**
 * Change selected tracks length
 */
void changeTrackLength(byte const value)
{
  if (state.track > 0)
  {
    byte trackIndex = MAX_TRACKS - 1;
    do
    {
      if (state.isTrackSelected(tracks[trackIndex]->trackID()))
      {
        LOG("changeTrackLength to %d of tracks[%d]", value, trackIndex);
        int newValue = (tracks[trackIndex]->getPatternSize() + value) % MAX_STEP;
        tracks[trackIndex]->setTrackLength(newValue);
      }
    } while (trackIndex--);
  }
}

void changeNotesDuration(byte const trackIndex)
{
  tracks[trackIndex]->changeNotesDuration();
}

void updateNotesDuration(byte const trackIndex)
{
  tracks[trackIndex]->updateNotesDuration();
}

/**
 * setup code to run once
 */
void setup()
{
  // link each tracks to the global noteOffHandler
  byte trackIndex = MAX_TRACKS - 1;
  do
  {
    tracks[trackIndex] = new Track(trackIndex, &noteOffHandler);
  } while (trackIndex--);

  RK002_clockSetMode(1); // internal clock only
  RK002_clockSetTempo(RK002_paramGet(BPM) * 10);
  LOG("Setup function called, channel:%d", RK002_paramGet(MIDICHN));
}

void loop()
{
  // put your main code here, to run repeatedly:
}
