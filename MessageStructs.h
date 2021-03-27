#ifndef MSG_STRUCT_LIB_
#define MSG_STRUCT_LIB_
#include <stdint.h>

struct message_header {
  uint32_t payload_size;
  uint32_t msg_id;
};
struct message {
  struct message_header hdr;
  uint8_t payload[0];
};
struct button_event_data {
  uint32_t type;
  uint32_t btn_idx;
  uint32_t resp_req;
};
struct display_size {
  uint32_t length;
  uint32_t width;
  uint32_t height;
};
struct set_register {
  uint32_t addr;
  uint32_t value;
};
struct rtc_time {
  uint32_t hours;
  uint32_t mins;
  uint32_t secs;
};


#endif