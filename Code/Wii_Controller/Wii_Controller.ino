
#include "My_NRF.h"
#include "Controller.h"

Controller Wii;
My_NRF Sender;
stControllerStatus CurrentStatus;
bool isAckReceived;

void setup() {
  // put your setup code here, to run once:

  bool IsWiiOk;
  bool IsNrfOk;
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH);
  IsWiiOk = Wii.Init();
  IsNrfOk = Sender.Init();
  if(IsNrfOk && IsWiiOk)
  {
    digitalWrite(STATUS_LED_PIN, LOW);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  CurrentStatus = Wii.Update();

  do
  {
    Sender.SendStatus(CurrentStatus);

    isAckReceived = Sender.WaitForAck();
  }while(!isAckReceived);

  #ifdef DEBUG_RF
    Serial.println("ACK Recieved!");
  #endif

  delay(500);
}
