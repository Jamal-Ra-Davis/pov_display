
#include <Servo.h>
#define MAX_ERROR 7500
#define SPEED_CTRL_PIN  9
#define LED_PIN         30
#define POT_PIN         A6
Servo myservo;          // create servo object to control a servo

unsigned long prev_us;
long int targ_us = 20000;
volatile unsigned long cnt = 0;
float Kp = 0.001;
float Ki = 0.0;
float Kd = -0.002;
float f_val;
long curr_us = 0;
bool init_complete = false;
long int error_i = 0;
long int prev_error = 0;

long int t_del = 0;
bool data_ready = false;

static inline int update_targ_us(long int t_)
{
  static const unsigned long MAX_US = 100000;
  static const unsigned long MIN_US = 16666;

  if (t_ > MAX_US || t_ < MIN_US || t_ == targ_us)
  {
    return -1;
  }
  targ_us = t_;
  error_i = 0;
  return 0;
}
void setup_pc_interrupt()
{
  cli();
  PCMSK0 |= (1 << PCINT7);
  PCICR = 0x01;
  sei();
}
void setup() {
  Serial.begin(115200);
  
  myservo.attach(SPEED_CTRL_PIN, 1000, 2000);  // attaches the servo on pin 9 to the servo object
  myservo.write(0);
  
  prev_us = micros();
  setup_pc_interrupt();

  //Bring motor up to min speed
  int start_up = 0;
  while (cnt < 10)
  {
    int val = map(start_up, 0, 1023, 0, 180); 
    myservo.write(val);
    start_up += 2;
    if (start_up > 200)
    {
      break;
    }
    delay(100);
  }
  f_val = start_up;
  init_complete = true;
}

int top = 0;
void loop() {
  if (data_ready)
  {
    data_ready = false;

    long int error = targ_us - t_del;
    if (error > MAX_ERROR)
      error = MAX_ERROR;
    else if (error < -MAX_ERROR)
      error = -MAX_ERROR; 

    if (error_i > 500000)
      error_i = 500000;
    else if (error_i < -500000)
      error_i = -500000;

    long int error_del = error - prev_error;
    float change = -1*Kp*error + -1*Ki*error_i + Kd*error_del;

    /*
    if (change > 40)
    {
      change = 40;
    }
    else if (change < -40)
    {
      change = -40;
    }
    */
    
    f_val += change;
    if (f_val > 1023)
    {
      f_val = 1023;
    }
    if (f_val < 100)
    {
      f_val = 100;
    }
  
    int esc_val = (int)(f_val + 0.5) + 1000;
    myservo.writeMicroseconds(esc_val);
    prev_error = error;
    error_i += error;

    Serial.print(t_del);
    Serial.print(" ");
    Serial.print(targ_us);
    Serial.print(" ");
    Serial.println();
  }
  
  if (Serial.available() > 0)
  {
    int temp = Serial.parseInt();
    Serial.print("Parsed value: ");
    Serial.println(temp);
    update_targ_us(temp);
  }
}

ISR(PCINT0_vect)
{
  //Check for falling edge
  if (PORTB & (1 << PB7))
  {
    return;
  }
  if (cnt % 2 == 1)
  {
    cnt++;
    return;
  }
  
  long int us = micros();
  t_del = us - prev_us;
  prev_us = us;
  cnt++;
  data_ready = true;
}
