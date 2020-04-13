#pragma once

//The following is the #include header guard.
#ifndef __TRACK_HPP__
#define __TRACK_HPP__

#include <RK002.h>
#include <RKKeyBuf.h>
#include "Pattern.hpp"
#include "NoteOffHandler.hpp"
#include "Constant.hpp"

class Track
{
private:
  byte _trackID;
  RKKeyBuf _buffer;
  NoteOffHandler *_bufferNoteOff;
  // check whether or not the track must records incomming notes
  // initialization to 0;
  Pattern _pattern;
  byte _trigs;
  bool _mode;
  byte _lastNoteIndex;
  byte _duration_mode;
  byte _channel;

public:
  Track() : _trackID(0),
            _buffer(MAX_BUFFER_SIZE),
            _pattern(),
            _trigs(0),
            _mode(MODE_CHORDS),
            _lastNoteIndex(0),
            _duration_mode(DURATION_PARAM_ONE),
            _channel(0){};
  Track(const byte trackID, NoteOffHandler *const bufferNoteOff);
  ~Track();

  inline void switchArpChordsMode() { _mode = !_mode; }
  void setTrackLength(byte const length);

  inline byte trackID() { return _trackID; }

  // pattern interface
  void increaseTrigs(byte const increment = 1);
  inline byte getTrigs() { return _trigs; }
  void setTrigs(byte const trigs);
  void clearTrigs();
  inline void setOffset(byte const offset) { _pattern.setOffset(offset); }
  inline void increaseOffset(byte const inc) { _pattern.increaseOffset(inc); }
  inline void decreaseOffset(byte const dec) { _pattern.decreaseOffset(dec); }
  inline bool isHit(byte const index) { return _pattern[index]; }
  inline void setHit(byte const index) { _pattern.setHit(index); }
  inline void setRest(byte const index) { _pattern.setRest(index); }
  inline byte getPatternSize() { return _pattern.size(); }
  inline void changeNotesDuration() { _duration_mode = (_duration_mode + 1) % DURATION_MODE; }
  void setPatternSize(const byte size);

  // notes handling
  void sendNotesOn(byte const index);
  //void sendNotesOff(byte const index);
  void sendAndPrepareNote(const byte clockIndex, const byte noteIndex);
  void addNoteOn(byte const key, byte const velocity);
  //void addNoteOff(byte const key, byte const index);
  byte relativeDuration(const byte a, const byte b);
  byte getDuration();
  void updateNotesDuration();
  //void prepareNoteOff(byte const  bufferIndex, byte const clockIndex);
  void clearNotes();
  void resetBufferNoteOff(byte const size);
};

Track::~Track()
{
}

Track::Track(const byte trackID, NoteOffHandler *const bufferNoteOff) : _trackID(trackID),
                                                                        _buffer(MAX_BUFFER_SIZE),
                                                                        _bufferNoteOff(bufferNoteOff),
                                                                        _pattern(),
                                                                        _trigs(0),
                                                                        _mode(MODE_CHORDS),
                                                                        _lastNoteIndex(0),
                                                                        _duration_mode(4),
                                                                        _channel(0)
{
  // init bufferNoteOff
  LOG("Track %d constructor called", _trackID);
  LOG("\n _trigs:%d \n _mode:%d", _trigs, _mode);
  LOG("_lastNoteIndex:%d \n _duration_mode:%d \n_channel:%d", _lastNoteIndex, _duration_mode, _channel);
}

inline void Track::setTrackLength(byte const length)
{
  setPatternSize(length);
}

inline void Track::setPatternSize(const byte size)
{
  _pattern.setSize(size);
  resetBufferNoteOff(size);
}

inline void Track::setTrigs(byte const trigs)
{
  LOG("setTrigs(%d)", trigs);
  if (trigs > 0)
  {
    _trigs = trigs;
  }
  else
  {
    // if we setTrigs to 0
    // pattern must be cleared
    clearTrigs();
  }
}

inline void Track::clearTrigs()
{
  LOG("clearTrigs", true);
  _pattern.clear();
  _trigs = 0;
}

void Track::increaseTrigs(byte const increment)
{
  byte newTrigs = (_trigs + increment) % _pattern.size();
  setTrigs(newTrigs);
}

void Track::addNoteOn(byte const key, byte const velocity)
{
  _buffer.insert(key, velocity, getDuration());
}

byte Track::getDuration()
{
  byte duration = 1;
  switch (_duration_mode)
  {
  case DURATION_PARAM_RANDOM_ONCE:
  case DURATION_PARAM_RANDOM_EACH:
    duration = random(1, _pattern.size());
    /* code */
    break;
  case DURATION_PARAM_EUCLIDE:
    duration = (byte) (_pattern.size() / _trigs);
    /* code */
    break;
  case DURATION_PARAM_ONE:
  default:
    duration = 1;
    break;
  }
  return duration;
}

void Track::updateNotesDuration()
{
  for (int i = _buffer.size() - 1; i >= 0; i--)
  {
    _buffer[i]->userval = getDuration();
  }
}

byte Track::relativeDuration(byte const a, byte const b)
{
  byte _b = (b < a) ? b + _pattern.size() : b;
  return (_b - a) % _pattern.size();
}
/*
void Track::addNoteOff(byte const key, byte const index)
{
  // search for introduced note on
  RKKeyBufEntry *entry = _buffer.findByKey(key);
  if (entry)
  {
    // try to approximate a relative duration
    entry->userval = relativeDuration(index, entry->userval);
  }
}
*/
void Track::sendNotesOn(byte const index)
{
  // send notes
  if (_mode)
  {
    for (int noteIndex = 0; noteIndex < _buffer.size(); noteIndex++)
    {
      sendAndPrepareNote(index, noteIndex);
      LOG("Track::sendNotesOn (mode:%d) @index:%d for note:%d in the buffer", _mode, index, noteIndex);
      LOG("Track::sendNotesOn channel:%d, key:%d, vel:%d", _channel, _buffer[noteIndex]->key, _buffer[noteIndex]->vel);
      //RK002_sendNoteOn(_channel,_buffer[i]->key, _buffer[i]->vel);
      //prepareNoteOff(i,index);
    }
  }
  else
  { //arpeggio
    _lastNoteIndex = (1 + _lastNoteIndex) % _buffer.size();
    sendAndPrepareNote(index, _lastNoteIndex);
    LOG("Track::sendNotesOn (mode:%d) @index:%d for note:%d in the buffer", _mode, index, _lastNoteIndex);
    LOG("Track::sendNotesOn channel:%d, key:%d, vel:%d", _channel, _buffer[_lastNoteIndex]->key, _buffer[_lastNoteIndex]->vel);
    //RK002_sendNoteOn(_channel, _buffer[_lastNoteIndex]->key, _buffer[_lastNoteIndex]->vel);
    //prepareNoteOff(_lastNoteIndex, index);
  }
}

void Track::sendAndPrepareNote(const byte clockIndex, const byte noteIndex)
{
  LOG("Track::sendNotesOn @index:%d for note:%d in the buffer", clockIndex, noteIndex);
  LOG("Track::sendNotesOn channel:%d, key:%d, vel:%d", _channel, _buffer[noteIndex]->key, _buffer[noteIndex]->vel);
  // send note
  RK002_sendNoteOn(_channel, _buffer[noteIndex]->key, _buffer[noteIndex]->vel);
  // prepare Note Off
  // calculate the noteOff clockIndex based on the note's duration + clockIndex
  byte duration = _buffer[noteIndex]->userval;
  if (_duration_mode == DURATION_PARAM_RANDOM_EACH)
  {
    duration = getDuration();
  }
  byte noteOffIndex = (duration + clockIndex) % _pattern.size();
  // insert noteOff to be fired @ right index of the bufferNoteOff array
  _bufferNoteOff->storeNote(_buffer[noteIndex]->key, noteOffIndex);
}

void Track::resetBufferNoteOff(byte const size)
{
  for (int pos = 0; pos < MAX_STEP; pos++)
  {
    _bufferNoteOff->flushNote(pos, _channel);
  }
}

void Track::clearNotes()
{
  // send notes Off for all notes On
  for (int noteIndex = 0; noteIndex < _buffer.size(); noteIndex++)
  {
    RK002_sendNoteOff(_channel, _buffer[noteIndex]->key, _buffer[noteIndex]->vel);
  }
  _buffer.removeAll();
  // in case of...
  resetBufferNoteOff(_pattern.size());
}

#endif
