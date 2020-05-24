#ifndef EVENTS_LIB
#define EVENTS_LIB

#include "RingBuf.h"



class Event;





class Event
{
  public:
    enum EVENT{ON_PRESS, ON_RELEASE};
  
    EVENT type;
    uint8_t button_idx;
    Event(){};
    Event(EVENT t, uint8_t b);
	
		static void SerialParser();
};
Event::Event(EVENT t, uint8_t b)
{
  type = t;
  button_idx = b;
}



extern RingBuf<Event, 32> eventBuffer;
RingBuf<Event, 32> eventBuffer;
char serialBuf[2];
int ser_idx = 0;

void Event::SerialParser()
{
  while(Serial1.available())
  {
    char c = Serial1.read();
    serialBuf[ser_idx%2] = c;
    ser_idx++;
    if (ser_idx >= 4)
      ser_idx -= 2;

    if (ser_idx >= 2)
    {
      if (serialBuf[1] >= 48 && serialBuf[1] <= 57)//Ascii: 0-9
      {
        int idx = serialBuf[1] - 48;
        if (serialBuf[0] == 'p')
        {
          eventBuffer.push(Event(Event::ON_PRESS, idx));
          SerialUSB.print("ON_PRESS: ");
          SerialUSB.println(idx);
        }
        if (serialBuf[0] == 'r')
        {
          eventBuffer.push(Event(Event::ON_RELEASE, idx));
          SerialUSB.print("ON_RELEASE: ");
          SerialUSB.println(idx);
        }
      }
    }
  }
}





#endif
