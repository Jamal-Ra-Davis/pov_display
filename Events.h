#ifndef EVENTS_LIB
#define EVENTS_LIB

#include "RingBuf.h"

//enum EVENT{B0_ON_PRESS, B0_ON_RELEASE, B1_ON_PRESS, B1_ON_RELEASE, ON_PRESS, ON_RELEASE};

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


void Event::SerialParser()
{
  while (Serial1.available() > 0)
  {
		char c = Serial1.read();
		int idx = Serial.parseInt();
		if (Serial.read() == '\n')
		{
			if (c == 'p')
				eventBuffer.push(Event(Event::ON_PRESS, idx));
			else if (c == 'r')
				eventBuffer.push(Event(Event::ON_RELEASE, idx));
		}
	}
}





#endif
