#include <RcRx.h>

#define CH1_PIN 16
#define CH2_PIN 17
#define LED_PIN 13
#define BUTTON_PIN 14

void update();
void btnIsr();

void setup()
{
  Serial.begin(9600);
  //delay(3000);
  Serial.print("Initializing RCRx\n");

  int chPins[] = {CH1_PIN, CH2_PIN};
  RCReceiver.init(2, chPins);
  RCReceiver.registerCallback(update);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, btnIsr, CHANGE);
}

void loop()
{
  //Serial.print("Polling Channels\n");

  int ch1 = RCReceiver.getFraction(0, 1000);
  int ch2 = RCReceiver.getFraction(1, 1000);
  //Serial.printf("Ch1: %d, Ch2: %d\n", ch1, ch2);

  Joystick.X(1000 - ch1);
  Joystick.Y(1000 - ch2);

  delay(25);
}

void btnIsr()
{
  //Serial.print("Button pressed, resetting channels\n");
  //RCReceiver.init(2, chPins);
  Joystick.button(1, 1 - digitalRead(BUTTON_PIN));
}

void update()
{
  Serial.print("New readings\n");
}

