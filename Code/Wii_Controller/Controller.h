
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Definies.h"
#include "Utils.h"

class Controller
{
  private:

    
    float X_value, X0, Xmin, Xmax;
    float Y_value, Y0, Ymin, Ymax;
    int C_button;
    int Z_button;
    float Status[4];

  float Convert(int analogValue)
  {
    float RetValue;

    RetValue = analogValue * AREF_VALUE / 1023.0;

    return RetValue;
  }

  void CalibrateCenter()
  {
    X0 = analogRead(X_ANALOG_PIN);
    Y0 = analogRead(Y_ANALOG_PIN);

    #ifdef DEBUG_WII
      Serial.print("X0:");
      Serial.print(X0);
      Serial.print("Y0:");
      Serial.println(Y0); 
    #endif
  }

  void SelfCalibration()
  {
    float JoyX = Status[X_STATUS_POSITION];
    Xmin = min(Xmin,JoyX);
    Xmax = max(Xmax,JoyX);

    float JoyY = Status[Y_STATUS_POSITION];
    Ymin = min(Ymin,JoyY);
    Ymax = max(Ymax,JoyY);
  }

  void ComputeStatus()
  {
      float JoyX = Status[X_STATUS_POSITION];
      float JoyY = Status[Y_STATUS_POSITION];
      SelfCalibration();

      int CenteredX = JoyX - X0;
      if(CenteredX == 0)
      {
        X_value = 0;
      }
      else if(CenteredX > 0)
      {
        X_value = 1.0 * CenteredX / (Xmax - X0);
      }
      else
      {
        X_value = -1.0 * CenteredX / (Xmin - X0);
      }

     int CenteredY = JoyY - Y0;
     if(CenteredY == 0)
      {
        Y_value = 0;
      }
      else if(CenteredY > 0)
      {
        Y_value = 1.0 * CenteredY / (Ymax - Y0);
      }
      else
      {
        Y_value = -1.0 * CenteredY / (Ymin - Y0);
      }

      Z_button = Status[Z_STATUS_POSITION];
      C_button = Status[C_STATUS_POSITION];
      
//      Serial.begin(9600);
//      
//      Serial.print("JoyX:");
//      Serial.println(JoyX);
//
//      Serial.print("X0:");
//      Serial.println(X0);
//      
//      Serial.print("X:");
//      Serial.println(X_value);
//      Serial.print("Y:");
//      Serial.println(Y_value, 4);
  }

  void ReadStatus()
  {
    Status[X_STATUS_POSITION] = (float)analogRead(X_ANALOG_PIN);
    Status[Y_STATUS_POSITION] = (float)analogRead(Y_ANALOG_PIN);
    Status[Z_STATUS_POSITION] = digitalRead(Z_BUTTON_PIN);
    Status[C_STATUS_POSITION] = digitalRead(C_BUTTON_PIN);

//    Serial.begin(9600);
//    
//    Serial.print("C:");
//    Serial.println(Status[C_STATUS_POSITION]);
  }
  
  public:

     bool Init()
    {

      this->Xmax = this->Ymax = 1023;
      this->Xmin = this->Ymin = 0;
      
      pinMode(C_BUTTON_PIN, INPUT);
      pinMode(Z_BUTTON_PIN, INPUT);
      pinMode(X_ANALOG_PIN, INPUT);
      pinMode(Y_ANALOG_PIN, INPUT);
      pinMode(X_ANALOG_PIN, INPUT);
      pinMode(BATT_ANALOG_PIN, INPUT);
      

      analogReference(EXTERNAL);

      #ifdef DEBUG_WII
        Serial.begin(9600);
      #endif

      CalibrateCenter();
      return true;
    }


    
    stControllerStatus Update()
    {
      stControllerStatus stRetValue;
      
      ReadStatus();
      ComputeStatus();

       #ifdef DEBUG_BATT
          Serial.print("Battery:");
          Serial.println(Convert(analogRead(BATT_ANALOG_PIN)));
        #endif
      
      if(Convert(analogRead(BATT_ANALOG_PIN)) <= BATT_VOLTAGE_LIMIT)//Battery voltage too low
      {
        digitalWrite(STATUS_LED_PIN, HIGH);
      }

      stRetValue.X = this->X_value;
      stRetValue.Y = this->Y_value;
      stRetValue.C = this->C_button;
      stRetValue.Z = this->Z_button;

      #ifdef DEBUG_WII
        Serial.print("X:");
        Serial.print(stRetValue.X);
        Serial.print(" ");
        Serial.print("Y:");
        Serial.print(stRetValue.Y);
        Serial.print(" ");
        Serial.print("C:");
        Serial.print(stRetValue.C);
        Serial.print(" ");
        Serial.print("Z:");
        Serial.println(stRetValue.Z);
        Serial.print(" ");
      #endif

      return stRetValue;
    }

    /*stControllerStatus GetStatus()
    {
      stControllerStatus stRetValue;

      stRetValue.X = X_value;
      stRetValue.Y = Y_value;
      stRetValue.C = C_button;
      stRetValue.Z = Z_button;

     return stRetValue;
    } */  
};
#endif

