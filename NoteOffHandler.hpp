#ifndef NOTE_OFF_HANDLER_HPP
#define NOTE_OFF_HANDLER_HPP

#include "Queue.hpp"
#include <RK002.h>
#include "Constant.hpp"

class NoteOffHandler
{
private:
  // fixed list
  // ptr to a Queue of reference to the note to Shut Off
  Queue _noteBuffer[MAX_STEP]{};
  // the number note inside the holding Queue
public:
  NoteOffHandler(/* args */);
  ~NoteOffHandler();
  // push noteOff to the right step index
  inline void storeNote(byte const key, byte const index) { _noteBuffer[index].push(key); }

  void flushNote(byte const index, byte channel);
};

NoteOffHandler::NoteOffHandler(/* args */)
{
}

NoteOffHandler::~NoteOffHandler()
{
}

/*
* Flush all notes @ a given index
*/
void NoteOffHandler::flushNote(byte const index, byte channel)
{
  if (_noteBuffer[index].count() > 0)
  {
    while (_noteBuffer[index].count() > 0)
    {
      byte key = _noteBuffer[index].pop();
      if (key > 0)
      {
        LOG("Flush Note number:%d @ index:%d (channel=%d, key=%d, 0)", _noteBuffer[index].count(), index, channel, key);
        RK002_sendNoteOff(channel, key, 0);
      }
      else
      {
        LOG("Nothing to flush @ index:%d, count:%d", index, _noteBuffer[index].count());
      }
    }
  }
}

#endif
