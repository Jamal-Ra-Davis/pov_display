#ifndef SHELL_LIB_
#define SHELL_LIB_

#include <Arduino.h>
#include "RingBuf.h"
#include "FrameBuffer.h"
#include "Events.h"
#include "MessageStructs.h"
#include <string.h>
#include <RTCZero.h>

struct message_header;

//TODO: Refactor message id #defines as enums
//Incoming Messages
#define GET_DISPLAY_SIZE        0
#define GET_BUFFER_TYPE         1
#define SET_BUFFER_TYPE         2
#define CLEAR_DISPLAY           3
#define UPDATE_DISPLAY          4
#define GET_PERIOD              5
#define BUTTON_EVENT            6
#define GET_REGISTER            7
#define SET_REGISTER            8
#define SET_MARQUEE_TEXT        9
#define USE_MARQEE              10
#define GET_RTC_TIME            11
#define SET_RTC_TIME            12
#define GET_EXEC_STATE          13
#define SET_EXEC_STATE          14
#define JOYSTICK_EVENT          15
#define TRIGGER_EVENT           16

//Outgoing Messages
#define ACK                     0
#define NACK                    1
#define GET_DISPLAY_SIZE_RESP   2
#define GET_BUFFER_TYPE_RESP    3
#define GET_PERIOD_RESP         4
#define LOG_MSG                 5
#define GET_REGISTER_RESP       6
#define GET_RTC_TIME_RESP       7
#define GET_EXEC_STATE_RESP     8

#define MAX_BUF_SZ 64
#define MAX_PAYLOAD (MAX_BUF_SZ-8)

typedef enum {
      TRIANGLE, SQUARE, CROSS, CIRCLE, 
      LBUMP, RBUMP, LSTICK, RSTICK, 
      SHARE, OPTIONS, DUP, DLEFT, 
      DDOWN, DRIGHT, DX, DY, NUM_KEYS
} ds4_keys_t;

typedef enum {
              POV_SCRATCH_LOOP,
              POV_TEST, 
              DS4_TEST,
              MAZE_GAME,
              SPACE_GAME,
              CLOCK_DISPLAY,
              NUM_POV_STATES
} pov_state_t;

extern int change_state(pov_state_t state);
extern pov_state_t exec_state;
extern RTCZero rtc;

extern char log_line_buf[MAX_PAYLOAD];
char log_line_buf[MAX_PAYLOAD];
#define LOG_POV_SHELL(shell, ...)                                               \
  ({                                                                            \ 
    snprintf(log_line_buf, MAX_PAYLOAD, __VA_ARGS__);                           \
    shell->send_data(LOG_MSG, (uint8_t*)log_line_buf, strlen(log_line_buf)+1);  \
  })                                                  

extern char serial_line_buf[MAX_BUF_SZ];
char serial_line_buf[MAX_BUF_SZ];
#define SERIAL_PRINTF(ser, ...)                             \
  ({                                                        \ 
    snprintf(serial_line_buf, MAX_BUF_SZ, __VA_ARGS__);     \
    ser.print(serial_line_buf);                             \
  })  

extern doubleBuffer frame_buffer;
extern long timer_delta;

class Shell {
  private:
    RingBuf<uint8_t, 256> ring_buf;
    uint8_t end_buf[2];
    uint8_t msg_ready;
     
    int read_ringbuf(uint8_t* dst, size_t N);
    int execute_command(struct message* msg);
    
    
  public:
    static const uint8_t MSG_FT0 = 0xBE;
    static const uint8_t MSG_FT1 = 0xEF; 
    Shell();
    void reset();
    int receive_data(uint8_t val);
    int parse_data(); 
    int send_data(uint16_t msg_id, uint8_t* payload, size_t payload_size);
    uint8_t get_ready_messages() {return msg_ready;}
       
};

Shell::Shell()
{
  msg_ready = 0;
  ring_buf.clear();
}
void Shell::reset()
{
  msg_ready = 0;
  ring_buf.clear();
}
int Shell::receive_data(uint8_t val)
{
  static int end_idx = 0;
  if (end_idx < 2)
  {
    end_buf[end_idx++] = val;
  }
  else
  {
    end_buf[0] = end_buf[1];
    end_buf[1] = val;
    if (end_buf[0] == MSG_FT0 && end_buf[1] == MSG_FT1)
    {
      end_idx = 0;
      msg_ready++;
    }
  }
  ring_buf.push(val);
}
int Shell::parse_data()
{
  uint8_t end_id[2];
  static uint8_t msg_buf[128];
  struct message* msg = (struct message*)msg_buf;
  if (read_ringbuf((uint8_t*)msg, sizeof(struct message_header)) != 0)
  {
    //Failed to read, invalid size
    msg_ready = 0;
    ring_buf.clear();   
    return -1;
  }

  if (read_ringbuf(msg->payload, msg->hdr.payload_size) != 0)
  {
    //Failed to read, invalid size
    msg_ready = 0;
    ring_buf.clear();   
    return -1;
  }

  if (read_ringbuf(end_id, 2) != 0)
  {
    //Failed to read, invalid size
    msg_ready = 0;
    ring_buf.clear();   
    return -1;
  }

  execute_command(msg);

  if (msg_ready > 0)
  {
    msg_ready--;
  }
}
int Shell::read_ringbuf(uint8_t* dst, size_t N)
{
  if (N == 0)
  {
    return 0;
  }
  if (ring_buf.size() < N)
  {
    return -1;
  }
  
  for (int i=0; i<N; i++)
  {
    ring_buf.pop(dst[i]);
  }
  return 0;
}



int Shell::execute_command(struct message *msg)
{
  int ret;
  bool msg_handled = false;

  if (msg == NULL)
  {
    send_data(NACK, NULL, 0);
    return -1;
  }

  switch(msg->hdr.msg_id)
  {
    case GET_DISPLAY_SIZE:
    {
      struct display_size disp_sz = {
        .length = LENGTH,
        .width = WIDTH,
        .height = HEIGHT,
      };

      ret = send_data(GET_DISPLAY_SIZE_RESP, (uint8_t*)&disp_sz, sizeof(struct display_size));
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to send GET_DISPLAY_SIZE response");
        break;
      }
      char buf[MAX_BUF_SZ];
      snprintf(buf, MAX_BUF_SZ, "L: %d, W: %d, H: %d", disp_sz.length, disp_sz.width, disp_sz.height);
      SerialUSB.println(buf);
      msg_handled = true;
      break;
    }

    case GET_BUFFER_TYPE:
    {
      bool single_buffer = frame_buffer.isSingleBuffered();

      ret = send_data(GET_BUFFER_TYPE_RESP, (uint8_t*)&single_buffer, sizeof(bool));
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to send GET_BUFFER_TYPE response");
        break;
      }
      char buf[MAX_BUF_SZ];
      snprintf(buf, MAX_BUF_SZ, "SingleBuffer: %d", (int)single_buffer);
      SerialUSB.println(buf);
      msg_handled = true;
      break;
    }

    case SET_BUFFER_TYPE:
    {
      if (msg->hdr.payload_size < sizeof(bool))
      {
        break;
      }
      bool *single_buffer = (bool*)msg->payload;
      if (*single_buffer == true)
        frame_buffer.forceSingleBuffer();
      else
        frame_buffer.forceDoubleBuffer();
      
      ret = send_data(ACK, NULL, 0);
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK SET_BUFFER_TYPE");
        break;
      }
      SERIAL_PRINTF(SerialUSB, "Buffer type set: %d", (int)(*single_buffer));
      msg_handled = true;
      break;
    }

    case CLEAR_DISPLAY:
    {
      if (msg->hdr.payload_size < sizeof(bool))
      {
        SerialUSB.println("Error: Failed to execute CLEAR_DISPLAY");
        break;
      }
      bool *update = (bool*)msg->payload;
      frame_buffer.clear();
      if (*update)
      {
        frame_buffer.update();
      }

      ret = send_data(ACK, NULL, 0);
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK CLEAR_DISPLAY");
        break;
      }
      char buf[MAX_BUF_SZ];
      snprintf(buf, MAX_BUF_SZ, "Frame buffer cleared, update: %d", (int)(*update));
      SerialUSB.println(buf);
      msg_handled = true;
      break;
    }

    case UPDATE_DISPLAY:
    {
      frame_buffer.update();

      ret = send_data(ACK, NULL, 0);
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK UPDATE_DISPLAY");
        break;
      }
      char buf[MAX_BUF_SZ];
      snprintf(buf, MAX_BUF_SZ, "Frane buffer updated");
      SerialUSB.println(buf);
      msg_handled = true;
      break;
    }

    case GET_PERIOD:
    {
      long per = timer_delta;

      ret = send_data(GET_PERIOD_RESP, (uint8_t*)&per, sizeof(long));
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK GET_PERIOD");
        break;
      }
      char buf[MAX_BUF_SZ];
      snprintf(buf, MAX_BUF_SZ, "Period: %d", (int)per);
      SerialUSB.println(buf);
      msg_handled = true;
      break;
    }

    case BUTTON_EVENT:
    {
      
      if (msg->hdr.payload_size < sizeof(struct button_event_data))
      {
        SerialUSB.println("Error: Failed to execute BUTTON_EVENT");
        break;
      }
      struct button_event_data *data = (struct button_event_data*)msg->payload;
      if (data->type >= Event::NUM_BTN_EVENTS)
      {
        SerialUSB.println("Error: Failed to execute BUTTON_EVENT");
        break;
      }

      eventBuffer.push(Event((Event::EVENT)(data->type), data->btn_idx));
      if (data->resp_req)
      {
        ret = send_data(ACK, NULL, 0);
        if (ret != 0)
        {
          SerialUSB.println("Error: Failed to ACK BUTTON_EVENT");
          break;
        }
      }
      char buf[MAX_BUF_SZ];
      snprintf(buf, MAX_BUF_SZ, "Button Event. Button Idx: %d, Button Type: %d", data->btn_idx, data->type);
      SerialUSB.println(buf);
      msg_handled = true;
      break;
    }

    case GET_REGISTER:
    {
      if (msg->hdr.payload_size < sizeof(uint32_t))
      {
        SerialUSB.println("Error: Failed to execute GET_REGISTER");
        break;
      }
      uint32_t *addr_ = (uint32_t*)msg->payload;
      uint32_t addr = (*addr_);
      uint32_t reg_val = *((uint32_t*)addr);

      ret = send_data(GET_REGISTER_RESP, (uint8_t*)&reg_val, sizeof(uint32_t));
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK GET_REGISTER");
        break;
      }
    
      SERIAL_PRINTF(SerialUSB, "Reg[%08X]: %08X\n", addr, reg_val);
      msg_handled = true;
      break;
    }

    case SET_REGISTER:
    {
      if (msg->hdr.payload_size < sizeof(struct set_register))
      {
        SerialUSB.println("Error: Failed to execute SET_REGISTER");
        break;
      }

      struct set_register *set_reg = (struct set_register*)msg->payload;
      *((uint32_t*)set_reg->addr) = set_reg->value;

      ret = send_data(ACK, NULL, 0);
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK SET_REGISTER");
        break;
      }
    
      SERIAL_PRINTF(SerialUSB, "Reg[%08X]: %08X\n", set_reg->addr, set_reg->value);
      msg_handled = true;
      break;
      
    }

    case SET_MARQUEE_TEXT:
    {
      break;
    }

    case USE_MARQEE:
    {
      break;
    }

    case GET_RTC_TIME:
    {
      struct rtc_time rtc_time = {
        .hours = rtc.getHours(),
        .mins = rtc.getMinutes(),
        .secs = rtc.getSeconds(),
      };

      ret = send_data(GET_RTC_TIME_RESP, (uint8_t*)&rtc_time, sizeof(struct rtc_time));
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK GET_RTC_TIME");
        break;
      }

      SERIAL_PRINTF(SerialUSB, "RTC Time = (%02d:%02d:%02d)\n", rtc_time.hours, rtc_time.mins, rtc_time.secs);
      msg_handled = true;
      break;
    }

    case SET_RTC_TIME:
    {
      if (msg->hdr.payload_size < sizeof(struct rtc_time))
      {
        break;
      }
      struct rtc_time *rtc_time = (struct rtc_time*)msg->payload;

      rtc.setTime(rtc_time->hours, rtc_time->mins, rtc_time->secs);

      ret = send_data(ACK, NULL, 0);
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK SET_RTC_TIME");
        break;
      }
      SERIAL_PRINTF(SerialUSB, "RTC Time = (%02d:%02d:%02d)\n", rtc_time->hours, rtc_time->mins, rtc_time->secs);
      msg_handled = true;
      break;
    }

    case GET_EXEC_STATE:
    {
      uint32_t state = (uint32_t)exec_state;

      ret = send_data(GET_EXEC_STATE_RESP, (uint8_t*)&state, sizeof(uint32_t));
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK GET_EXEC_STATE");
        break;
      }
      
      SERIAL_PRINTF(SerialUSB, "Current state: %d\n", state);
      msg_handled = true;
      break;
    }

    case SET_EXEC_STATE:
    {
      if (msg->hdr.payload_size < sizeof(uint32_t))
      {
        SerialUSB.println("Error: Failed to execute SET_EXEC_STATE");
        break;
      }
      uint32_t *state = (uint32_t*)msg->payload;
      ret = change_state((pov_state_t)(*state));
      if (ret != 0)
      {
        SerialUSB.println("Error: Error occured while changing state");
        break;
      }

      ret = send_data(ACK, NULL, 0);
      if (ret != 0)
      {
        SerialUSB.println("Error: Failed to ACK SET_EXEC_STATE");
        break;
      }
    
      SERIAL_PRINTF(SerialUSB, "Updated state: %d\n", (*state));
      msg_handled = true;
      break;
    }

    case JOYSTICK_EVENT:
    {
      
      if (msg->hdr.payload_size < sizeof(struct joystick_event_data))
      {
        SerialUSB.println("Error: Failed to execute JOYSTICK_EVENT");
        break;
      }
      struct joystick_event_data *data = (struct joystick_event_data*)msg->payload;
      if (data->type != Event::L_STICK && data->type != Event::R_STICK)
      {
        SerialUSB.println("Error: Failed to execute JOYSTICK_EVENT");
        break;
      }

    Event e = Event::createJoystickEvent((Event::ABS_TYPE)data->type, 
                                          (int16_t)data->x, 
                                          (int16_t)data->y,
                                          (uint16_t)data->angle,
                                          (uint8_t)data->mag);
      eventBuffer.push(e);
      if (data->resp_req)
      {
        ret = send_data(ACK, NULL, 0);
        if (ret != 0)
        {
          SerialUSB.println("Error: Failed to ACK JOYSTICK_EVENT");
          break;
        }
      }
      char buf[MAX_BUF_SZ];
      snprintf(buf, MAX_BUF_SZ, "Joystick Event. X: %d, Y: %d", data->x, data->x);
      SerialUSB.println(buf);
      msg_handled = true;
      break;
    }

    case TRIGGER_EVENT:
    {
      
      if (msg->hdr.payload_size < sizeof(struct trigger_event_data))
      {
        SerialUSB.println("Error: Failed to execute TRIGGER_EVENT");
        break;
      }
      struct trigger_event_data *data = (struct trigger_event_data*)msg->payload;
      if (data->type != Event::L_TRIG && data->type != Event::R_TRIG)
      {
        SerialUSB.println("Error: Failed to execute TRIGGER_EVENT");
        break;
      }

      eventBuffer.push(Event::createTriggerEvent((Event::ABS_TYPE)data->type, data->trigger));
      if (data->resp_req)
      {
        ret = send_data(ACK, NULL, 0);
        if (ret != 0)
        {
          SerialUSB.println("Error: Failed to ACK TRIGGER_EVENT");
          break;
        }
      }
      char buf[MAX_BUF_SZ];
      snprintf(buf, MAX_BUF_SZ, "Trigger Event. Trigger: %u", data->trigger);
      SerialUSB.println(buf);
      msg_handled = true;
      break;
    }

    default:
    {
      break;
    }
  }
  if (!msg_handled)
  {
    send_data(NACK, NULL, 0);
  }
  return 0;
}

int Shell::send_data(uint16_t msg_id, uint8_t* payload, size_t payload_size)
{
  uint8_t buf[MAX_BUF_SZ + 2];
  struct message *msg = (struct message*)buf;

  msg->hdr.msg_id = msg_id;
  msg->hdr.payload_size = payload_size;
  if (payload_size > 0)
  {
    memcpy(msg->payload, payload, payload_size);
  }
  msg->payload[payload_size] = MSG_FT0;
  msg->payload[payload_size+1] = MSG_FT1;
  
  Serial1.write(buf, sizeof(message_header) + payload_size + 2);
  for (int i=0; i < (sizeof(message_header) + payload_size + 2); i++)
  {
    SerialUSB.print("Sending: ");
    SerialUSB.println(buf[i], HEX);
  }
  return 0;
}

int shell_testing(Shell *shell)
{
  int i;
  if (shell == NULL)
  {
    return -1;
  }

  SerialUSB.println("Shell function validation...");

  shell->reset();

  //Setup message
  SerialUSB.println("Setting up GET_DISPLAY_SIZE message");
  uint8_t buf[MAX_BUF_SZ];
  struct message *msg = (struct message*)buf;
  msg->hdr.payload_size = 0;
  msg->hdr.msg_id = GET_DISPLAY_SIZE;

  size_t msg_size = sizeof(struct message_header) + msg->hdr.payload_size;
  for (i=0; i<msg_size; i++)
  {
    shell->receive_data(buf[i]);
  }
  shell->receive_data(Shell::MSG_FT0);
  shell->receive_data(Shell::MSG_FT1);

  //Check if messages are ready
  int num_msgs = shell->get_ready_messages();
  if (num_msgs < 1)
  {
    SerialUSB.println("Error: no messages ready");
    return -1;
  }

  SerialUSB.println("At least 1 message ready for parsing");
  for (i=0; i < num_msgs; i++)
  {
    if (shell->parse_data() < 0)
    {
      SerialUSB.print("Error parsing msg #");
      SerialUSB.println(i);
      return -1;
    }
  }

  SerialUSB.println("Parsed all ready messages");

  SerialUSB.println("Setting up GET_BUFFER_TYPE message");
  msg->hdr.payload_size = 0;
  msg->hdr.msg_id = GET_BUFFER_TYPE;

  msg_size = sizeof(struct message_header) + msg->hdr.payload_size;
  for (i=0; i<msg_size-3; i++)
  {
    shell->receive_data(buf[i]);
  }
  //Check number of messages
  num_msgs = shell->get_ready_messages();
  SerialUSB.print(num_msgs);
  SerialUSB.println(" messages ready. Should be 0");
  int i_ = i;

  Serial.println("Parsing buffer, should do nothing");
  for (i=0; i < num_msgs; i++)
  {
    if (shell->parse_data() < 0)
    {
      SerialUSB.print("Error parsing msg #");
      SerialUSB.println(i);
      return -1;
    }
  }

  //Filling out rest of message
  for (i=i_; i<msg_size; i++)
  {
    shell->receive_data(buf[i]);
  }
  shell->receive_data(Shell::MSG_FT0);
  shell->receive_data(Shell::MSG_FT1);


  SerialUSB.println("Setting up BUTTON_EVENT message");
  msg->hdr.payload_size = sizeof(struct button_event_data);
  msg->hdr.msg_id = BUTTON_EVENT;
  struct button_event_data *btn_data = (struct button_event_data*)msg->payload;
  btn_data->btn_idx = 1;
  btn_data->type = Event::TAP; 
  msg_size = sizeof(struct message_header) + msg->hdr.payload_size;

  for (i=0; i<msg_size; i++)
  {
    shell->receive_data(buf[i]);
  }
  shell->receive_data(Shell::MSG_FT0);
  shell->receive_data(Shell::MSG_FT1);

  num_msgs = shell->get_ready_messages();
  SerialUSB.print(num_msgs);
  SerialUSB.println(" messages ready. Should be 2");

  SerialUSB.println("At least 1 message ready for parsing");
  for (i=0; i < num_msgs; i++)
  {
    if (shell->parse_data() < 0)
    {
      SerialUSB.print("Error parsing msg #");
      SerialUSB.println(i);
      return -1;
    }
  }

  SerialUSB.println("Parsed all ready messages");

  return 0;
}
#endif