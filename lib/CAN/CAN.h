#ifndef _CAN_H
#define _CAN_H

void setup_can();
void sendCANMessage(int id, int data0, int data1, int data2, int data3, int data4, int data5, int data6,int data7);

#endif
