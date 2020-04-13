#pragma once

//The following is the #include header guard.
#ifndef __State_HPP__
#define __State_HPP__

#include <Arduino.h>
#include <RK002.h>
#include "Constant.hpp"

class State
{

public:
  State();
  ~State();
  // not so orthodoxa
  byte track;
  byte trig;
  byte mode;
  byte bpm;

  inline bool isTrackSelected(const byte idTrack) { return bitRead(track, idTrack); }
  inline bool isModeActive(const byte idMode) { return bitRead(mode, idMode); }
  inline bool trigsChanged(const byte idTrack) { return bitRead(trig, idTrack); }
  inline int optionFactor() { return bitRead(bpm, OPTION_PARAM_10)?100:10; }
  inline bool trigsChangesExists()
  {
    LOG("modifyedTrigsExists:[%d,%d,%d,%d]", trigsChanged(ID_TRACKS_0),
        trigsChanged(ID_TRACKS_1),
        trigsChanged(ID_TRACKS_2),
        trigsChanged(ID_TRACKS_3));
    return trig > 0;
  }
};

State::State() : track(0), trig(0), mode(0), bpm(0)
{
}

State::~State()
{
}

#endif