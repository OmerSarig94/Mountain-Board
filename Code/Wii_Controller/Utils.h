
#ifndef UTILS_H
#define UTILS_H

#define COMMAND_DATA_SIZE 10
#define BARKER1_POSITION  0
#define BARKER2_POSITION  1
#define BARKER3_POSITION  2
#define X_SYMBOL_POSITION 3
#define X_VALUE_POSITION  4
#define Y_SYMBOL_POSITION 5
#define Y_VALUE_POSITION  6
#define C_STATUS_POSITION 7
#define Z_STATUS_POSITION 8
#define XOR_POSITION      9

#define ACK_PACKET_SIZE 7

#define ABS(X) (X > 0 ? X : -X)

typedef struct 
{
  float X;
  float Y;
  int C;
  int Z; 
}stControllerStatus;

void PrintBuffer(uint8_t* pucBuffer, uint8_t ucBufferSize)
{
  for(int i=0;i<ucBufferSize;i++)
  {
    Serial.print(pucBuffer[i],DEC);
    Serial.print(" ");
  }
  Serial.println();
}


#endif
