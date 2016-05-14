
#ifndef MY_NRF_H
#define MY_NRF_H

#include <SPI.h>
#include <RH_NRF24.h>
#include "Definies.h"
#include "Utils.h"

#define ACK_TIMOUT  500

class My_NRF
{
  private:

    const String ACK = "MBT_ACK";
    uint8_t pucCommand[COMMAND_DATA_SIZE];
    uint8_t AckBuff[ACK_PACKET_SIZE];
    RH_NRF24 nrf24;

    uint8_t CheckXor(uint8_t* pucBuffer, uint8_t ucSize)
    {
      uint8_t RetValue = 0;
      for(int i=0; i<ucSize;i++)
      {
        RetValue = RetValue ^ pucBuffer[i];
      }

      return RetValue;
    }

    int StatusToOpcode(int Button)
    {
      if(Button == HIGH)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }

    bool CheckBarker(uint8_t* pucAckBuff)
    {
      bool bRetValue = false;
      if((pucAckBuff[BARKER1_POSITION] == 'M') &&
          (pucAckBuff[BARKER2_POSITION] == 'B') &&
          (pucAckBuff[BARKER3_POSITION] == 'T'))
      {
         bRetValue = true;   
      }

      return bRetValue;
    }
  
    void BuildCommand(uint8_t* pucCommand, stControllerStatus Status)
    {
      uint8_t pucStatus[COMMAND_DATA_SIZE];

      //Add the barker
      pucStatus[BARKER1_POSITION] = 'M';
      pucStatus[BARKER2_POSITION] = 'B';
      pucStatus[BARKER3_POSITION] = 'T';

      //Insert X value to array
      pucStatus[X_VALUE_POSITION] = ABS(Status.X) * 100;
      pucStatus[X_SYMBOL_POSITION] = Status.X > 0 ? 1:0;

      //Insert Y value to array
      pucStatus[Y_VALUE_POSITION] = ABS(Status.Y) * 100;
      pucStatus[Y_SYMBOL_POSITION] = Status.Y > 0 ? 1:0;

      //Insert C and Z values to array
      pucStatus[C_STATUS_POSITION] = StatusToOpcode(Status.C);
      pucStatus[Z_STATUS_POSITION] = StatusToOpcode(Status.Z);

      //Compute and Insert the XOR
      pucStatus[XOR_POSITION] = CheckXor(pucStatus, COMMAND_DATA_SIZE-1);// the -1 is because we compute the xor so the xor pos is not in the array

      //copy the array to the OUT array
      memcpy(pucCommand, pucStatus, COMMAND_DATA_SIZE);
    }
    
  public:

    bool Init()
    { 
      bool bNrfInit;
      bool bChannel;
      bool bSetRF;
      bNrfInit = nrf24.init();
      bChannel = nrf24.setChannel(1);
      bSetRF = nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm);
      
      #ifdef DEBUG_RF
      Serial.begin(9600);
      if(!bNrfInit)
      {
        Serial.println("init failed");
      }
      if(!bChannel)
      {
        Serial.println("setChannel failed");
      }
      if(!bSetRF)
      {
        Serial.println("setRF failed");
      }
      #endif

      if((!bNrfInit) ||
          (!bChannel)||
          (!bSetRF))
      {
        return false;     
      }
      else
      {
        return true;
      }
    }
    
    void SendStatus(stControllerStatus Status)
    {
      BuildCommand(this->pucCommand, Status);
      #ifdef DEBUG_RF
        PrintBuffer(this->pucCommand, sizeof(this->pucCommand));
      #endif
      nrf24.send(this->pucCommand, sizeof(this->pucCommand));
      nrf24.waitPacketSent();
    }

    bool WaitForAck()
    {
      uint8_t AckBuff[ACK_PACKET_SIZE];
      uint8_t len = sizeof(AckBuff);
      if(nrf24.waitAvailableTimeout(ACK_TIMOUT))
      {
        if(nrf24.recv(AckBuff, &len))
        {
          if(memcmp(AckBuff, &ACK, ACK_PACKET_SIZE) == 0)
          {
            return true;
          }

          return false;
        }
        
        return false;
      }

    }

   stControllerStatus WaitForCommand()
   {
      stControllerStatus stRetValue;
      uint8_t len = sizeof(pucCommand);
      if(nrf24.waitAvailableTimeout(ACK_TIMOUT))
      {
        if(nrf24.recv(pucCommand, &len))
        {
          if(CheckBarker(pucCommand))//check if barker exist
          {
            if(CheckXor(pucCommand, COMMAND_DATA_SIZE-1) == pucCommand[XOR_POSITION])//Compere the XORS
            {
              stRetValue.Z = pucCommand[Z_STATUS_POSITION];
              stRetValue.C = pucCommand[C_STATUS_POSITION];
              
             if(pucCommand[X_SYMBOL_POSITION] == 1)
             {
              stRetValue.X = -1 * pucCommand[X_VALUE_POSITION] / 100;
             }
             else
             {
              stRetValue.X = 1 * pucCommand[X_VALUE_POSITION] / 100;
             }

             if(pucCommand[Y_SYMBOL_POSITION] == 1)
             {
              stRetValue.Y = -1 * pucCommand[Y_VALUE_POSITION] / 100;
             }
             else
             {
              stRetValue.Y = 1 * pucCommand[Y_VALUE_POSITION] / 100;
             }
              

              return stRetValue;
            }
            else
            {
              #ifdef DEBUG_RF
                Serial.println("Wrong Xor:" + CheckXor(pucCommand, COMMAND_DATA_SIZE-1));
              #endif
            }
          }
          else
          {
            #ifdef DEBUG_RF
              Serial.println("Wrong Barker:" + pucCommand[BARKER1_POSITION] + pucCommand[BARKER2_POSITION] + pucCommand[BARKER3_POSITION]);
            #endif
          }

          #ifdef DEBUG_RF
            Serial.println("Recieve Error");
          #endif
          }

          #ifdef DEBUG_RF
            Serial.println("Timeout Error");
          #endif
        }
      }
};
#endif
