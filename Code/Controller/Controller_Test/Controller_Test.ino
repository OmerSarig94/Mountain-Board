
#include <SPI.h>
#include <RH_NRF24.h>

//pinout defines
#define C_BUTTON_PIN  7
#define Z_BUTTON_PIN  2
#define X_ANALOG_PIN  A0
#define Y_ANALOG_PIN  A1

#define AREF_VALUE 3.3

#define DEBUG


RH_NRF24 nrf24; //Instance of NRF

float Convert(float analogValue)
{
  float RetValue;

  RetValue = analogValue * AREF_VALUE / 1023.0;

  return RetValue;
}

void Update()
{
  float x_value, y_value;
  int c_status, z_status;

  z_status = digitalRead(Z_BUTTON_PIN);
  c_status = digitalRead(C_BUTTON_PIN);
  x_value = Convert(analogRead(X_ANALOG_PIN));
  y_value = Convert(analogRead(Y_ANALOG_PIN));
  
  #ifdef DEBUG
    Serial.print(x_value);
    Serial.print(",");
    Serial.println(y_value);
    if(z_status == HIGH)
    {
      Serial.println("Z Button pressed");
      //z_status = LOW;
    }

    if(c_status == HIGH)
    {
      Serial.println("C Button pressed");
      //c_status = LOW;
    }
   #endif
}

void setup() 
{

  bool isNrfInit;
  bool isCurrectChannel;
  bool isNrfSet;
  
  // put your setup code here, to run once:
  pinMode(C_BUTTON_PIN, INPUT);
  pinMode(Z_BUTTON_PIN, INPUT);
  pinMode(X_ANALOG_PIN, INPUT);
  pinMode(Y_ANALOG_PIN, INPUT);

  analogReference(EXTERNAL);

  isNrfInit = nrf24.init();
  isCurrectChannel = nrf24.setChannel(1);
  isNrfSet = nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm);
  
 #ifdef DEBUG
  Serial.begin(9600);
  if(isNrfInit == false)
  {
    Serial.println("NRF24 init failed!!");
  }
  if(isCurrectChannel == false)
  {
    Serial.println("NRF24 Channel Incorrect!!");
  }
  if(isNrfSet == false)
  {
    Serial.println("NRF24 sets incorrectly!!");
  }
 #endif
  
}

void loop() 
{
  // put your main code here, to run repeatedly:
  Update();
  delay(500);
  

}
