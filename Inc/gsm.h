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
#include "stm32f1xx_hal.h"

/** 
  * @brief 
  */ 
typedef struct
{
  char imei[15];
	char rssi[5];
}GSMTypeDef;

// Инициализация GSM
//------------------
void GSM_Init(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart);

void Send2Site(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart);

#endif
