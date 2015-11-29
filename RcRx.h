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

#ifndef RC_RX_H
#define RC_RX_H

#include "Arduino.h"

typedef struct {
  int pin;
  int minVal;
  int maxVal;
  int data;
  int prevData;
  int timeStamp;
  int prevPinState;
  int riseInterval;
  int updated;
} rc_channel;

class RCRx
{
  public:
    RCRx();
    void init(int noChan, int chPins[]);
    void registerCallback(void(*func)(void));
    int getChannel(int ch);
    int getFraction(int ch, int scale);

    void chIsr(int ch);
  private:
    void update(int ch);

    rc_channel * channels;
    int nChannels;
    void(*callback)(void);
};

extern RCRx RCReceiver;

#endif
