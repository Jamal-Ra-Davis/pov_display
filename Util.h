#ifndef POV_UTIL_LIB
#define POV_UTIL_LIB

#include <Arduino.h>
#include "DMA_SPI.h"

#define LED_PIN 9
#define HALL_PIN 6
#define CPU_HZ 48000000
#define CPU_HZ_DIV 6000000
#define TIMER_PRESCALER_DIV 1024

#define CPU_HZ_SCALE 48ull
#define CPU_HZ_SCALE_DIV 6ull

#define MAX_PERIOD_US 100000ULL
#define MIN_PERIOD_US 16666ULL



void setTimerFrequency(int frequencyHz);
void setTimerPeriod(int period_us);
void startTimer(int frequencyHz);
void startTimerPeriod(int period_us);
void TC3_Handler();
inline void adjust_timing(TcCount16* TC, long timer_temp, long *timer_0);
void hallTrigger();
void processSerialCommands(Shell *shell);
void sercomSetup(SPIClass *mySPI);
void hallEffectSetup();

extern long timer_0;
extern long timer_delta;



void setTimerFrequency(int frequencyHz) {
  //int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  int compareValue = (CPU_HZ_DIV / frequencyHz) - 1;
  TcCount16* TC = (TcCount16*) TC3;
  
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}
void setTimerPeriod(int period_us) {//period in us
  int compareValue = (CPU_HZ_SCALE_DIV*period_us) - 1;
  TcCount16* TC = (TcCount16*) TC3;
  
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void startTimer(int frequencyHz) {
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
  while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync

  TcCount16* TC = (TcCount16*) TC3;

  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Set prescaler to 1024
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV8;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  setTimerFrequency(frequencyHz);

  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC3_IRQn);
  NVIC_SetPriority(TC3_IRQn, 4); 

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}

void startTimerPeriod(int period_us) {
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
  while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync

  TcCount16* TC = (TcCount16*) TC3;

  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Set prescaler to 1024
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV8;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  setTimerPeriod(period_us);

  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC3_IRQn);
  NVIC_SetPriority(TC3_IRQn, 4); 

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}

void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  // If this interrupt is due to the compare register matching the timer count
  // we toggle the LED.
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1;
    // Write callback here!!!
    if (digitalRead(HALL_PIN))
      digitalWrite(LED_PIN, LOW);
    if (buf_idx >= LENGTH)
        return;
    if (!transfer_complete)
    {
      buf_idx++;

      //If interrupt triggers before transfer can complete, still prepare next DMA array
      //so next slice will display correctly
      int next_idx = buf_idx % LENGTH;
      convert_fb_to_dma(next_idx, buf_offset, frame_buffer.getReadBuffer(), pArray_next);
      return;
    }
  
    //driveLEDS(buf_idx, (int*)buf_offset, &frame_buffer, &mySPI);
    start_dma_transaction();
    buf_idx++;
    
    int next_idx = buf_idx % LENGTH;
    convert_fb_to_dma(next_idx, buf_offset, frame_buffer.getReadBuffer(), pArray_next);
  }
}

inline void adjust_timing(TcCount16* TC, long timer_temp, long *timer_0)
{
  timer_delta = timer_temp - *timer_0;
  *timer_0 = timer_temp;
  if (timer_delta < MIN_PERIOD_US)
  {
    timer_delta = MIN_PERIOD_US;
  }
  else if (timer_delta > MAX_PERIOD_US)
  {
    timer_delta = MAX_PERIOD_US;
  }
  
  timer_delta -= (timer_delta >> 8);
  uint16_t period_us = (timer_delta/LENGTH);
  uint16_t compareValue = (CPU_HZ_SCALE_DIV*period_us) - 1;
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}
void hallTrigger()
{
  digitalWrite(LED_PIN, HIGH);

  //Reset timer to zero
  TcCount16* TC = (TcCount16*) TC3;
  TC->COUNT.reg = 0;
  long timer_temp = micros();
  buf_idx = 1;
  
  if (!transfer_complete)
  {
    //Update timing even if, DMA transaction not finished
    adjust_timing(TC, timer_temp, &timer_0);
  
    //If interrupt triggers before transfer can complete, still prepare next DMA array
    //so next slice will display correctly
    convert_fb_to_dma(buf_idx, buf_offset, frame_buffer.getReadBuffer(), pArray_next);
    return;
  }
  
  start_dma_transaction();
  convert_fb_to_dma(buf_idx, buf_offset, frame_buffer.getReadBuffer(), pArray_next);

  adjust_timing(TC, timer_temp, &timer_0);
}

void processSerialCommands(Shell *shell)
{
  //Capture Serial Data if Available
  while(Serial1.available() > 0)
  {
    uint8_t val = Serial1.read();
    SerialUSB.print("Received: ");
    SerialUSB.println(val, HEX);
    shell->receive_data(val);
  }

  //Parse Messages if Available
  for (int i=0; i < shell->get_ready_messages(); i++)
  {
    if (shell->parse_data() < 0)
    {
      SerialUSB.print("Error: Failed to parse #");
      SerialUSB.println(i);
      break;
    }
  }
}

void sercomSetup(SPIClass *mySPI)
{
  mySPI->begin();

  // Assign pins 11, 12, 13 to SERCOM functionality
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
}

void hallEffectSetup()
{
  //Setup hall effect
  pinMode(HALL_PIN, INPUT);
  //hall = digitalRead(HALL_PIN); // Don't think is necessary
  timer_delta = 0;
  timer_0 = micros();
  
  attachInterrupt(HALL_PIN, hallTrigger, FALLING);
  delay(5);// Allow time for timer delta to setup
}

#endif