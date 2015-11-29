/*
RCRx
Copyright (C) 2015 Marcus Ahlberg

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Arduino.h"
#include "RCRx.h"


////////////////////////
// Defines
////////////////////////
#define MAX_CHANNELS 6

#define IRS_N(N) static void chIsr ## N(void) { chIsr(N); }

//static void chIsr(int ch);

//ISR_N(0);
//ISR_N(1);
//ISR_N(2);
//ISR_N(3);
//ISR_N(4);
//ISR_N(5);
static void chIsr0(void) { RCReceiver.chIsr(0); }
static void chIsr1(void) { RCReceiver.chIsr(1); }
static void chIsr2(void) { RCReceiver.chIsr(2); }
static void chIsr3(void) { RCReceiver.chIsr(3); }
static void chIsr4(void) { RCReceiver.chIsr(4); }
static void chIsr5(void) { RCReceiver.chIsr(5); }


////////////////////////
// Types
////////////////////////



////////////////////////
// Local variables
////////////////////////
void (*ch_isr[])() = {
  chIsr0,
  chIsr1,
  chIsr2,
  chIsr3,
  chIsr4,
  chIsr5,
};

RCRx RCReceiver;


////////////////////////
// Public interface
////////////////////////
RCRx::RCRx()
{
}

void RCRx::init(int noChan, int chPins[])
{
  nChannels = noChan;
  channels = (rc_channel *)malloc(sizeof(rc_channel) * nChannels);
  for (int ch = 0; ch < nChannels; ++ch)
  {
    channels[ch].pin = chPins[ch];
    channels[ch].minVal = 1400;
    channels[ch].maxVal = 1600;
    channels[ch].data = 0;
    channels[ch].prevData = 0;
    channels[ch].timeStamp = 0;
    channels[ch].prevPinState = 0;
    channels[ch].riseInterval = 0;
    channels[ch].updated = 0;

    pinMode(channels[ch].pin, INPUT);
    attachInterrupt(channels[ch].pin, ch_isr[ch], CHANGE);
  }
}

void RCRx::registerCallback(void(*func)(void))
{
  callback = func;
}
int RCRx::getChannel(int ch)
{
  return channels[ch].data;
}

int RCRx::getFraction(int ch, int scale)
{
  const int val = channels[ch].data;

  // Sanity check 1000-2000 with 10% margin
  if (val < 900 || val > 2200)
    return 0;

  if (val > channels[ch].maxVal)
    channels[ch].maxVal = val;
  if (val < channels[ch].minVal)
    channels[ch].minVal = val;

  return (scale * (val - channels[ch].minVal)) / (channels[ch].maxVal - channels[ch].minVal);
}


////////////////////////
// Private functions
////////////////////////
void RCRx::update(int ch)
{
  bool allUpdated = true;
  channels[ch].updated = 1;
  for (int i = 0; i < nChannels; ++i)
    if (channels[ch].updated)
      allUpdated = false;

  if (allUpdated)
    callback();
}


////////////////////////
// Interrupt handler
////////////////////////
void RCRx::chIsr(int ch)
{
  const int pinState = digitalRead(channels[ch].pin);
  const int curTime = micros();
  const int prevTime = channels[ch].timeStamp;
  channels[ch].timeStamp = curTime;

  // Sanity checks

  // Pin state same as previous
  if (pinState == channels[ch].prevPinState)
  {
    //Serial.printf("ERROR: %u to %u\n", prevPinState, pinState);
  }
  channels[ch].prevPinState = pinState;

  if (HIGH == pinState)
  {
    const int prevInterval = channels[ch].riseInterval;
    const int curInterval = curTime - prevTime;
    const int q = 100 * abs(curInterval - prevInterval) / prevInterval;
    channels[ch].riseInterval = curInterval;
    
    // Change greater than X percent
    if (q > 3)
    {
      //Serial.printf("ERROR: Interval diff %d %%, prev: %d, cur: %d\n", q, prevInterval, curInterval);
      return;
    }
    
    // Check if interval is at least 19 ms
    if (curInterval < 19000)
    {
      // error
      //Serial.printf("ERROR: Too short interval: %d\n", curInterval);
      return;
    }

    // Alles gut, save data
    channels[ch].riseInterval = curInterval;
    channels[ch].timeStamp = curTime;
  }
  else
  {
    const int curData = curTime - prevTime;
    const int q = 100 * abs(curData - channels[ch].prevData) / channels[ch].prevData;
    channels[ch].prevData = curData;
    if (q > 5)
    {
      //ERROR
      //Serial.printf("ERROR: Too large change: %d %%\n", q);
      return;
    }

    channels[ch].data = curData;
    update(ch);
  }
}
