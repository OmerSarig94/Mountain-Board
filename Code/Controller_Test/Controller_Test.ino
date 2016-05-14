
#include <SPI.h>
#include <RH_NRF24.h>

//pinout defines
#define C_BUTTON_PIN  7
#define Z_BUTTON_PIN  2
#define X_ANALOG_PIN  A0
#define Y_ANALOG_PIN  A1

#define AREF_VALUE 3.3

#define DATA_PACKET_SIZE 10
#define ACK_PACKET_SIZE 7

#define DEBUG_CONTROLLER
#define DEBUG_RF


RH_NRF24 nrf24; //Instance of NRF

uint8_t pucMessage[DATA_PACKET_SIZE];

uint8_t CheckXor(uint8_t* pucBuffer)
{
  uint8_t RetValue = 0;
  for(int i=0; i<DATA_PACKET_SIZE-1;i++)
  {
    RetValue = RetValue ^ pucBuffer[i];
  }

  return RetValue;
}

void PrintBuffer(uint8_t* pucBuffer)
{
  for(int i=0;i<DATA_PACKET_SIZE;i++)
  {
    Serial.print(pucBuffer[i],DEC);
    Serial.print(" ");
  }
  Serial.println();
}

float Convert(int analogValue)
{
  float RetValue;

  RetValue = analogValue * AREF_VALUE / 1023.0;

  return RetValue;
}

void Update(uint8_t* pucMessageBuff)
{
  int x_value, y_value;
  int c_status, z_status;

  uint8_t pucInnerMessage[DATA_PACKET_SIZE];

  pucInnerMessage[0] = 'M';
  pucInnerMessage[1] = 'B';
  pucInnerMessage[2] = 'T';
  
  z_status = digitalRead(Z_BUTTON_PIN);
  c_status = digitalRead(C_BUTTON_PIN);
  x_value = analogRead(X_ANALOG_PIN);
  y_value = analogRead(Y_ANALOG_PIN);

  pucInnerMessage[3] = x_value/100;
  pucInnerMessage[4] = (x_value) % (100);

 pucInnerMessage[5] = y_value/100;
  pucInnerMessage[6] = (y_value) % (100);
  
  if(z_status == HIGH)
  {
    pucInnerMessage[7] = 1;
  }
  else
  {
    pucInnerMessage[7] = 0;
  }

  if(c_status == HIGH)
  {
    pucInnerMessage[8] = 1;
  }
  else
  {
    pucInnerMessage[8] = 0;
  }

  pucInnerMessage[9] = CheckXor(pucInnerMessage);
  
  #ifdef DEBUG_CONTROLLER
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

   #ifdef DEBUG_RF
    PrintBuffer(pucInnerMessage);
   #endif

   memcpy(pucMessageBuff, pucInnerMessage, DATA_PACKET_SIZE);
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
  
 #ifdef DEBUG_RF
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

 #ifdef DEBUG_CONTROLLER
  Serial.begin(9600);
 #endif
  
}

void loop() 
{
  // put your main code here, to run repeatedly:
  Update(pucMessage);

  nrf24.send(pucMessage, DATA_PACKET_SIZE);

  nrf24.waitPacketSent();

  uint8_t AckBuff[ACK_PACKET_SIZE];
  uint8_t len = ACK_PACKET_SIZE;

  if (nrf24.waitAvailableTimeout(700))
  {
    // Should be a reply message for us now
    if (nrf24.recv(AckBuff, &len))
    {
      #ifdef DEBUG_RF
        Serial.println((char*)AckBuff);
      #endif
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is nrf24_server running?");
  }
  
  delay(10000);
  

}
