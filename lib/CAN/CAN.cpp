#include "STM32_CAN.h"

STM32_CAN Can( CAN1, DEF );  //Use PA11/12 pins for CAN1.
//STM32_CAN Can( CAN1, ALT );  //Use PB8/9 pins for CAN1.
//STM32_CAN Can( CAN1, ALT_2 );  //Use PD0/1 pins for CAN1.
//STM32_CAN Can( CAN2, DEF );  //Use PB12/13 pins for CAN2.
//STM32_CAN Can( CAN2, ALT );  //Use PB5/6 pins for CAN2
//STM32_CAN Can( CAN3, DEF );  //Use PA8/15 pins for CAN3.
//STM32_CAN Can( CAN3, ALT );  //Use PB3/4 pins for CAN3

static CAN_message_t CAN_TX_msg;
static CAN_message_t CAN_RX_msg;



void setup_can(){

    Can.begin();
    //Can.setBaudRate(250000);  //250KBPS
    // Can.setBaudRate(500000);  //500KBPS
    Can.setBaudRate(1000000);  //1000KBPS
  
}

void sendCANMessage(int id, int data0, int data1, int data2, int data3, int data4, int data5, int data6,int data7)
{
    // Exemple : Envoi d'un message CAN
    CAN_RX_msg.id = id; // ID CAN
    CAN_RX_msg.len = 8; // DLC : Nombre d'octets dans le message
    CAN_RX_msg.buf[0] = data0;      // Données a envoyés
    CAN_RX_msg.buf[1] = data1;
    CAN_RX_msg.buf[2] = data2;
    CAN_RX_msg.buf[3] = data3;
    CAN_RX_msg.buf[4] = data4;
    CAN_RX_msg.buf[5] = data5;
    CAN_RX_msg.buf[6] = data6;
    CAN_RX_msg.buf[7] = data7;

    Can.write(CAN_TX_msg);

}

