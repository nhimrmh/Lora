/*
 * mainApp.c
 *
 *  Created on: Sep 8, 2017
 *      Author: dkhairnar
 */

#include "mainApp.h"
#include "sx1276_7_8.h"
#include "string.h"
#define printUSB(x) CDC_Transmit_FS((uint8_t*)x,strlen((char*)x))

char strBuf[128];
u16 SysTime;
u16 time2_count;
u16 key1_time_count;
u16 key2_time_count;
u8 rf_rx_packet_length;

u8 mode;//lora--1/FSK--0
u8 Freq_Sel;//
u8 Power_Sel;//
u8 Lora_Rate_Sel;//
u8 BandWide_Sel;//
uint32_t count = 0; 
uint32_t number_of_ack = 0;
uint32_t number_of_noack = 0;
u8 Fsk_Rate_Sel;//
u8 count_idx = 1;
u8 flag_idx = 0;
s8 RSSI_Array[3][3];
u8 count_1 = 0;
u8 count_2 = 0;
u8 count_3 = 0;

u8 key1_count;
/*key1_count = 0----------->lora master
key1_count = 1----------->lora slaver
*/
u8 time_flag;
/*{
bit0 time_1s;
bit1 time_2s;
bit2 time_50ms;
bit3 ;
bit4 ;
bit5 ;
bit6 ;
bit7 ;
}*/
u8	operation_flag;
/*typedef struct
{
	uchar	:RxPacketReceived-0;
	uchar	:
	uchar	:
	uchar	:
	uchar	:
	uchar	:key2_down;
	uchar	:key1_down;
	uchar	;
} operation_flag;*/
u8 key_flag;
/*{
	uchar	:key1_shot_down;
	uchar	:key1_long_down;
	uchar	:key2_short_down;
	uchar	:key2_long_down
	uchar	:
	uchar	:;
	uchar	:;
	uchar	;
}*/
uint32_t tick_temp;
void mainApp()
{
	u16 i=0;//,j,k=0,g;

	SysTime = 0;
	operation_flag = 0x00;
	key1_count = 0x00;
	mode = 0x01;//lora mode
	Freq_Sel = 0x00;//433M
	Power_Sel = 0x00;//
	Lora_Rate_Sel = 0x00;// Spreading Factor config, 0x00 = 6
	BandWide_Sel = 0x09;
	Fsk_Rate_Sel = 0x00;
	
	//RED_LED_L();
	//HAL_Delay(500);
	//RED_LED_H();

	HAL_GPIO_WritePin(Reset_GPIO_Port,Reset_Pin,GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(Reset_GPIO_Port,Reset_Pin,GPIO_PIN_SET);
	
	/*
	Init RSSI table with all values are 0
	*/
	for(uint32_t i = 0; i < 3; i++){
		for(uint32_t j = 0; j < 3; j++){
			RSSI_Array[i][j] = 0;
		}
	}
	/*
	Choose mode
	*/
	key1_count = 0;
	/*
	First initialization for Lora 
	*/
	sx1276_7_8_Config();                                         //setting base parameter
	sx1276_7_8_LoRaEntryRx();
	while (1)
	{		
		switch(key1_count)
		{

			case 0://lora master Tx
				{
					//HAL_Delay(100000);
					u8 Tx_Packet[20];
					sprintf((char*)sx1276_7_8Data,"2_%d\n", i++);		
					sprintf((char*)Tx_Packet, "Data sent: %s\n", (char*)sx1276_7_8Data);
					printUSB(Tx_Packet);
					sx1276_7_8_LoRaEntryTx();
							
					sx1276_7_8_LoRaTxPacket();	
					//sx1276_7_8_Standby();//Entry Standby mode			
					sx1276_7_8_LoRaEntryRx();
					key1_count = 1;					
				}		
			break;
				
			case 1://lora slaver Rx continuous
			{					
					if(sx1276_7_8_LoRaRxPacket())
					{								
						if(strncmp((char*)RxData,"2",1) == 0){							
							s8 rssi_value = sx1276_7_8_LoRaReadRSSI();
							sprintf(strBuf,"Data received: %s\n",(char*)RxData);					
							printUSB(strBuf);			
							//sx1276_7_8_Standby();//Entry Standby mode												
							key1_count = 0;
						}			
					}
					else{
													
					}
				}		
			break;
									
			case 6:				
				//HAL_Delay(10000);
				tick_temp = HAL_GetTick();
				sprintf((char*)sx1276_7_8Data,"%d_hahaha\n",count_idx);		
				count_idx++;
				if(count_idx == 4){
					count_idx = 1;
				}									
				//sprintf((char*)temp_s,"\nData sent: %s", (char*)sx1276_7_8Data);
				//RED_LED_H();
				sx1276_7_8_LoRaEntryTx();
				HAL_Delay(10);
				sx1276_7_8_LoRaTxPacket();	
				key1_count = 7;
				sx1276_7_8_LoRaEntryRx();														
				
			break;
				
			case 7:
				count = 0;						
				while(count < 1500000){
					if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == 1) {
						RED_LED_L();
						count ++;
					}
					else{
						RED_LED_H();
						count ++;
					}
					
					if(count == 1500000 - 1) {						
						if(count_idx-1 == 1){
							RSSI_Array[0][count_1] = 0;
							count_1++;
							if(count_1 == 3) count_1 = 0;
							number_of_noack++;
							sprintf(strBuf,"\n ID  1   2   3 \n    ___________\n\n 1 |%3d %3d %3d|\n 2 |%3d %3d %3d|\n 3 |%3d %3d %3d|\n    ___________\n",
							RSSI_Array[0][0],RSSI_Array[0][1],RSSI_Array[0][2], RSSI_Array[1][0],RSSI_Array[1][1],RSSI_Array[1][2]
							,RSSI_Array[2][0],RSSI_Array[2][1],RSSI_Array[2][2]);				
							printUSB(strBuf);		
						}
						
						if(count_idx-1 == 2){
							RSSI_Array[1][count_2] = 0;
							count_2++;
							if(count_2 == 3) count_2 = 0;
							number_of_noack++;
							sprintf(strBuf,"Receive: %d", result);
							printUSB(strBuf);		
						}
						
						else if(count_idx-1 == 0){
							RSSI_Array[2][count_3] = 0;
							count_3++;
							if(count_3 == 3) count_3 = 0;
							number_of_noack++;
							sprintf(strBuf,"\n ID  1   2   3 \n    ___________\n\n 1 |%3d %3d %3d|\n 2 |%3d %3d %3d|\n 3 |%3d %3d %3d|\n    ___________\n",
							RSSI_Array[0][0],RSSI_Array[0][1],RSSI_Array[0][2], RSSI_Array[1][0],RSSI_Array[1][1],RSSI_Array[1][2]
							,RSSI_Array[2][0],RSSI_Array[2][1],RSSI_Array[2][2]);				
							printUSB(strBuf);		
						}	
						
					}		
					else{
						                  //RegDioMapping2 DIO5=00, DIO4=01
					}
					if(sx1276_7_8_LoRaRxPacket()){	
						if(strncmp((char*)RxData,"1",1) == 0){							
							s8 rssi_value = sx1276_7_8_LoRaReadRSSI();
							count = 1500000;
							number_of_ack++;
							RSSI_Array[0][count_1] = rssi_value;
							count_1++;
							if(count_1 == 3) count_1 = 0;
							
							sprintf(strBuf,"\n ID  1   2   3 \n    ___________\n\n 1 |%3d %3d %3d|\n 2 |%3d %3d %3d|\n 3 |%3d %3d %3d|\n    ___________\n",
							RSSI_Array[0][0],RSSI_Array[0][1],RSSI_Array[0][2], RSSI_Array[1][0],RSSI_Array[1][1],RSSI_Array[1][2]
							,RSSI_Array[2][0],RSSI_Array[2][1],RSSI_Array[2][2]);				
							printUSB(strBuf);	
						}
						else if(strncmp((char*)RxData,"2",1) == 0){	
							s8 save[5];
							
							strncpy((char*)save, (char*)RxData + 2, strcspn((char*)RxData, "&") - 2);
							u8 idx = 2 - 1;
							//char* temp = substring((char*)RxData, 2, strcspn((char*)RxData, "&"));
							//memcpy(save, &temprx[2], strcspn((char*)RxData, "&") - 2);
							s8 rssi_value = sx1276_7_8_LoRaReadRSSI();
							count = 1500000;
							number_of_ack++;
							/*
							RSSI_Array[0][idx] = save;						
							sprintf(strBuf,"\n ID  1   2   3 \n    ___________\n\n 1 |%3d %3d %3d|\n 2 |%3d %3d %3d|\n 3 |%3d %3d %3d|\n    ___________\n",
							RSSI_Array[0][0],RSSI_Array[0][1],RSSI_Array[0][2], RSSI_Array[1][0],RSSI_Array[1][1],RSSI_Array[1][2]
							,RSSI_Array[2][0],RSSI_Array[2][1],RSSI_Array[2][2]);				
							*/
						
						}	
						else if(strncmp((char*)RxData,"3",1) == 0){							
							s8 rssi_value = sx1276_7_8_LoRaReadRSSI();
							count = 1500000;
							number_of_ack++;
							RSSI_Array[2][count_3] = rssi_value;
							count_3++;
							if(count_3 == 3) count_3 = 0;
							sprintf(strBuf,"\n ID  1   2   3 \n    ___________\n\n 1 |%3d %3d %3d|\n 2 |%3d %3d %3d|\n 3 |%3d %3d %3d|\n    ___________\n",
							RSSI_Array[0][0],RSSI_Array[0][1],RSSI_Array[0][2], RSSI_Array[1][0],RSSI_Array[1][1],RSSI_Array[1][2]
							,RSSI_Array[2][0],RSSI_Array[2][1],RSSI_Array[2][2]);				
							printUSB(strBuf);	
						}	
					}
					else{
						//SPIWrite(REG_LR_DIOMAPPING2,0x05);                     //RegDioMapping2 DIO5=00, DIO4=01
					}
					
				}
							
				key1_count = 6;
				
				sx1276_7_8_Standby(); 
			break;
				
		}
	}
}
