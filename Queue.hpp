/*
 * Byte Queue.h
 * 
 * Based on Steven de Salas
 * 
 * Defines a class for a queue of byte.
 * Used for Arduino projects, just #include "Queue.h" and add this file via the IDE.
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <Arduino.h>
#include "Constant.hpp"

class Queue
{
private:
  byte _front;
  byte _back;
  byte _count;
  byte _data[MAX_ELEMENT_SIZE + 1]{0};
  byte const _maxitems = MAX_ELEMENT_SIZE;

public:
  Queue() : _front(0), _back(0), _count(0)
  {
  }

  ~Queue()
  {
    //delete[] _data;
  }

  inline byte count() { return _count; }
  inline byte front() { return _front; }
  inline byte back() { return _back; }
  void push(const byte &item);
  byte peek();
  byte pop();
  void clear();
};

void Queue::push(const byte &item)
{
  LOG("Queue::push(%d) called front:%d, count:%d", item, _front, _count);
  if (_count < _maxitems)
  { // Drops out when full
    _data[_back++] = item;
    ++_count;
    // Check wrap around
    if (_back > _maxitems)
    {
      _back -= (_maxitems + 1);
    }
  }
  LOG("Q:push(%d) front:%d, count:%d", item, _front, _count);
}

byte Queue::pop()
{
  byte result = 0;
  if (_count <= 0)
  {
    LOG("Queue::pop() empty, result:%d", result);
  }
  else
  {
    LOG("Queue::pop() called front:%d, count:%d", _front, _count);
    result = _data[_front];
    _front++;
    --_count;
    // Check wrap around
    if (_front > _maxitems)
    {
      _front -= (_maxitems + 1);
    }
    LOG("Q:pop() front:%d, count:%d, result:%d", _front, _count, result);
  }
  return result;
}

byte Queue::peek()
{
  byte result;
  if (_count > 0)
  {
    result = _data[_front];
  }
  return result;
}

void Queue::clear()
{
  _front = _back;
  _count = 0;
}

#endif
