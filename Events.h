#ifndef EVENTS_LIB
#define EVENTS_LIB

#include "RingBuf.h"

class Event
{
  public:
    enum EVENT{ON_PRESS, ON_RELEASE, TAP, NUM_BTN_EVENTS};
  
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


void process_serial_commands(doubleBuffer* frame_buffer)
{
  static RingBuf<char, 32> serialBuffer;
  while (Serial1.available())
  {
    char c = Serial1.read();
    SerialUSB.print((int)c);
    SerialUSB.print(" ");
    if (c == '\n')
    {
      //Parse when find newline character
      SerialUSB.println();
      serialBuffer.pop(c);
      int bytes_remaining = serialBuffer.size();
      switch(c)
      {
        case 'c':
        {
          frame_buffer->clear();
          SerialUSB.println("c");
          break;
        }
        case 'u':
        {
          frame_buffer->update();
          SerialUSB.println("u");
          break;
        }
        case 's':
        {
          char parse_buf[32] = {'\0'};
          int parse_idx = 0;
          for (parse_idx = 0; parse_idx < bytes_remaining; parse_idx++)
          {
            serialBuffer.pop(parse_buf[parse_idx]);
          }
          parse_buf[parse_idx] = '\0';

          int x, y, z, r, g, b;
          sscanf(parse_buf, "%d %d %d %d %d %d", &x, &y, &z, &r, &g, &b);
          char command[32];
          sprintf(command, "s %d %d %d %d %d %d\n", x, y, z, (uint8_t)r, (uint8_t)g, (uint8_t)b);
          SerialUSB.print(command);
          frame_buffer->setColors(x, y, z, r, g, b);
          break;
        }
        case 'p':
        case 'r':
        {
          char state = c;
          serialBuffer.pop(c);
          int idx = (int)c - 48;

          SerialUSB.print("Button Action: ");
          SerialUSB.print(state);
          SerialUSB.println(idx);
          
          if (state == 'p')
          {
            eventBuffer.push(Event(Event::ON_PRESS, idx));
            SerialUSB.print("ON_PRESS: ");
            SerialUSB.println(idx);
          }
          else if (state = 'r')
          {
            eventBuffer.push(Event(Event::ON_RELEASE, idx));
            SerialUSB.print("ON_RELEASE: ");
            SerialUSB.println(idx);
          }
          break;
        }
        default:
        {
          //Don't recognize first character of packet, clear and move on to next
          serialBuffer.clear();
        }
      }
    }
    else if (c != 0)
    {
      serialBuffer.push(c);
      //SerialUSB.print(c);
    }
  }
}





#endif
