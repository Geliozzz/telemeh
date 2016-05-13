/****************************************************************
*****************                            ********************
*****************************************************************
*  UART.                                                        *
* ------------------------------------------------------------- *
*  АВТОР: Почепцов Олег Анатольевич								  					  *
* ------------------------------------------------------------- *
*****************************************************************/
#include <string.h>
#include "uart.h"

UART_HandleTypeDef *ptr_huart;
uint8_t rx_buff[SIZE_RXBUFF];
uint8_t* rx_buff_HEAD;
uint8_t* rx_buff_TAIL;


// Обработка прерывания по приему
//-------------------------------
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//Продолжаем прием по прерываниям
	HAL_UART_Receive_IT(huart, rx_buff, SIZE_RXBUFF);
}
// Обработка прерывания по ошибке
//-------------------------------
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	//Продолжаем прием по прерываниям
	HAL_UART_Receive_IT(huart, rx_buff, SIZE_RXBUFF);
}

// Инициализация UART
//------------------
void UART_Init(UART_HandleTypeDef *handle)
{
	// Передаем управляющую структуру
	ptr_huart = handle;
	rx_buff_HEAD = rx_buff;
	// Начать прием через прерывания
	HAL_UART_Receive_IT(ptr_huart, rx_buff, SIZE_RXBUFF);
}

// Приём символа через UART
// -----------------------
// - возвращает -1 если буфер приёма пуст, иначе принятый символ
//
int UART_getc( void )
{
	uint8_t c;
	static int i = 0;

	rx_buff_TAIL = ptr_huart->pRxBuffPtr;	
	if(rx_buff_TAIL >= rx_buff + SIZE_RXBUFF)
	{
		HAL_UART_Receive_IT(ptr_huart, rx_buff, SIZE_RXBUFF);
	}
	if(rx_buff_HEAD != rx_buff_TAIL)
	{
		c = *rx_buff_HEAD;

		// Кольцевой буфер
		rx_buff_HEAD++;
		if(rx_buff_HEAD >= rx_buff + SIZE_RXBUFF)
		{
			rx_buff_HEAD = rx_buff;
		}
		return c;
	}
	else
	{
		if( i )
		{
			i = 0;
		}
		return -1;
	}
}

// Передача строки из n символов через UART
// ---------------------------------------
// - возвращает 1 если строка помещена в буфер передачи, иначе 0
int UART_putn( const char * s, int n )
{
	HAL_UART_Transmit(ptr_huart, (uint8_t*)s, n, 100);
	return 1;
}
