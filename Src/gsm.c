/****************************************************************
*****************                            ********************
*****************************************************************
*  GSM.                                                         *
* ------------------------------------------------------------- *
*  АВТОР: Почепцов Олег Анатольевич	   												  *
* ------------------------------------------------------------- *
*****************************************************************/
#include "gsm.h"

#define WAIT_TIMEOUT 1000

char resp[50];

int GSM_WaitResp(void)
{
	uint32_t time_start, time_stop;
	int temp;
	int i = 0;
	
	time_start = HAL_GetTick();
	
	while(!strcmp(resp, "OK"))
	{
		temp = UART_getc();
		if(temp != -1)
		{
			resp[i++] = temp;
		}
		time_stop = HAL_GetTick();
		if((time_stop - time_start) > 500) return 0;
	}
	return 1;
}

void GSM_Init(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart)
{
	// Enable SIM900
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_Delay(2000);
	
	// Wait enable module
	
	HAL_UART_Transmit(user_uart, "Polus-IO test!", sizeof("Polus-IO test!"), 1000);
	//
	HAL_UART_Transmit(gsm_uart, "AT", sizeof("AT"), 1000);
	GSM_WaitResp();
	
	HAL_Delay(2000);
	//
	HAL_UART_Transmit(gsm_uart, "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", sizeof("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(gsm_uart, "AT+SAPBR=3,1,\"APN\",\"internet.mts.ru\"", sizeof("AT+SAPBR=3,1,\"APN\",\"internet.mts.ru\""), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(gsm_uart, "AT+SAPBR=3,1,\"USER\",\"mts\"", sizeof("AT+SAPBR=3,1,\"USER\",\"mts\""), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(gsm_uart, "AT+SAPBR=3,1,\"PWD\",\"mts\"", sizeof("AT+SAPBR=3,1,\"PWD\",\"mts\""), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(gsm_uart, "AT+SAPBR=1,1", sizeof("AT+SAPBR=1,1"), 1000);
	GSM_WaitResp();
	
	HAL_UART_Transmit(user_uart, "Checking for network...", sizeof("Checking for network..."), 1000);

//  while (fona.getNetworkStatus() != 1) 
//  {
//    delay(500);  
//  }

	HAL_UART_Transmit(gsm_uart, "AT+CMEE=2", sizeof("AT+CMEE=2"), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(user_uart, "RSSI:", sizeof("RSSI:"), 1000);
	//unsigned int RSSI1=fona.getRSSI();
	//  Serial.println(RSSI1);

	HAL_Delay(2000);
	HAL_UART_Transmit(user_uart, "Disabling GPRS", sizeof("Disabling GPRS"), 1000);
	//  fona.enableGPRS(false);
	HAL_Delay(5000);
	HAL_UART_Transmit(user_uart, "Enabling GPRS", sizeof("Enabling GPRS"), 1000);
	//while (!fona.enableGPRS(true)) 
//	{
	//  Serial.println(F("Failed to turn GPRS on"));
//	HAL_Delay(500);
//	//  fona.enableGPRS(false);      
//	HAL_Delay(500);
//  }
//   Watchdog.reset();
// //???????? ???? ??? ?????????? ????????????? ?????? 
//  memcpy(IMEI,0,sizeof(IMEI));
//  fona.getIMEI(IMEI);
	HAL_UART_Transmit(user_uart, "Connected to Cellular!", sizeof("Connected to Cellular!"), 1000);
	HAL_Delay(5000);
//  Watchdog.reset();
// timer.setInterval(5000, CheckiButton);
	
}

void Send2Site(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart)
{
	
}
