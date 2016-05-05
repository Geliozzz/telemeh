/****************************************************************
*****************                            ********************
*****************************************************************
*  GSM.                                                        *
* ------------------------------------------------------------- *
*  АВТОР: Почепцов Олег Анатольевич	   												  *
* ------------------------------------------------------------- *
*****************************************************************/
#ifndef GSM_H_
#define GSM_H_

#include "uart.h"
#include "string.h"
#include "stdlib.h"
#include "stm32f4xx_hal.h"

/** 
  */ 
typedef struct
{
  char imei[15];
	char rssi[5];
	int failtures;
}GSMTypeDef;

enum
{
	GSM_OK = 0,
	GSM_TIMEOUT
};

enum
{
	RESP_OK = 0,
	RESP_IMEI,
	RESP_RSSI,
	RESP_HTTPACTION
};

// Инициализация GSM
//------------------
void GSM_Init(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart);

void Send2Site(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart);

#endif
