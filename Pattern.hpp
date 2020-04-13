#pragma once

//The following is the #include header guard.
#ifndef __PATTERN_HPP__
#define __PATTERN_HPP__
#include <Arduino.h>
#include "Constant.hpp"

class Pattern
{
private:
  // number of step in the pattern
  byte _size = MAX_STEP;
  // the pattern array
  short _pattern = 0;
  // offset
  byte _offset = 0;

public:
  Pattern()
  {
    LOG("_pattern size %d bytes", sizeof(_pattern));
  };
  ~Pattern(){};

  inline void setSize(const byte size) { _size = constrain(size, 1, MAX_STEP); }
  inline byte size() { return _size; }

  bool operator[](byte index);

  inline void setOffset(const byte offset) { _offset = offset % _size; }
  void increaseOffset(const byte increment); // { _offset = (_offset + increment) % _size; }
  void decreaseOffset(const byte decrement); // { _offset = abs(_offset - decrement) % _size; }
  void setHit(const byte index);
  void setRest(const byte index);
  bool isHit(const byte index);
  inline void clear() { _pattern = 0; };
};

inline bool Pattern::operator[](const byte index)
{
  return isHit(index);
}

inline void Pattern::setHit(const byte index)
{
  bitSet(_pattern, index % _size);
}

inline void Pattern::setRest(const byte index)
{
  bitClear(_pattern, index % _size);
}

inline bool Pattern::isHit(const byte index)
{
  LOG("isHit? pattern:%d index:%d offset:%d size:%d", _pattern, index, _offset, _size);
  return bitRead(_pattern, (index + _offset) % _size);
}

inline void Pattern::increaseOffset(const byte increment)
{
  LOG("Inc Offset increment:%d offset:%d size:%d", increment, _offset, _size);
  _offset = constrain(_offset + increment, 0, _size);
  LOG("Inc Offset result offset=%d", _offset);
}

inline void Pattern::decreaseOffset(const byte decrement)
{
  LOG("Dec Offset decrement:%d offset:%d size:%d", decrement, _offset, _size);
  _offset = constrain(_offset - decrement, 0, _size);
  LOG("Dec Offset result offset=%d", _offset);
}

#endif