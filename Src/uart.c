/****************************************************************
*****************                            ********************
*****************************************************************
*  UART.                                                        *
* ------------------------------------------------------------- *
*  �����: �������� ���� �����������								  					  *
* ------------------------------------------------------------- *
*****************************************************************/
#include <string.h>
#include "uart.h"

UART_HandleTypeDef *ptr_huart;
uint8_t rx_buff[SIZE_RXBUFF];
uint8_t* rx_buff_HEAD;
uint8_t* rx_buff_TAIL;


// ��������� ���������� �� ������
//-------------------------------
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//���������� ����� �� �����������
	HAL_UART_Receive_IT(huart, rx_buff, SIZE_RXBUFF);
}
// ��������� ���������� �� ������
//-------------------------------
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	//���������� ����� �� �����������
	HAL_UART_Receive_IT(huart, rx_buff, SIZE_RXBUFF);
}

// ������������� UART
//------------------
void UART_Init(UART_HandleTypeDef *handle)
{
	// �������� ����������� ���������
	ptr_huart = handle;
	rx_buff_HEAD = rx_buff;
	// ������ ����� ����� ����������
	HAL_UART_Receive_IT(ptr_huart, rx_buff, SIZE_RXBUFF);
}

// ���� ������� ����� UART
// -----------------------
// - ���������� -1 ���� ����� ����� ����, ����� �������� ������
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

		// ��������� �����
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

// �������� ������ �� n �������� ����� UART
// ---------------------------------------
// - ���������� 1 ���� ������ �������� � ����� ��������, ����� 0
int UART_putn( const char * s, int n )
{
	HAL_UART_Transmit(ptr_huart, (uint8_t*)s, n, 100);
	return 1;
}
