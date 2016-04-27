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
#define URL "http://minachevamir.myjino.ru/rest/add.php?imei="

#define MAX_FAILURES 3

GSMTypeDef gsm;
char resp[50];

int GSM_WaitResp(void)
{
	char resp_ok[2];
	uint32_t time_start, time_stop;
	int temp;
	
	time_start = HAL_GetTick();
	
	while(!strcmp(resp_ok, "OK"))
	{
		temp = UART_getc();
		if(temp != -1)
		{
			resp_ok[0] = resp_ok[1];
			resp_ok[1] = temp;
		}
		time_stop = HAL_GetTick();
		if((time_stop - time_start) > WAIT_TIMEOUT) return GSM_TIMEOUT;
	}
	return GSM_OK;
}

/**
*/
int GSM_GetIMEI(UART_HandleTypeDef *gsm_uart)
{
	uint32_t time_start, time_stop;
	int temp;
	int i = 0;
	
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+GSN", sizeof("AT+GSN"), 1000);
	time_start = HAL_GetTick();
	while(i < sizeof(gsm.imei))
	{
		temp = UART_getc();
		if(temp != -1)
		{
			gsm.imei[i++] = temp;
		}
		time_stop = HAL_GetTick();
		if((time_stop - time_start) > WAIT_TIMEOUT) return GSM_TIMEOUT;
	}
	return GSM_WaitResp();
}

/**
*/
int GSM_GetRSSI(UART_HandleTypeDef *gsm_uart)
{
//	char string[20];
	char resp_ok[2];
	uint32_t time_start, time_stop;
	int temp;
//	int i = 0;
	
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+CSQ", sizeof("AT+CSQ"), 1000);
	time_start = HAL_GetTick();
	
	
	while(!strcmp(resp_ok, "OK"))
	{
		temp = UART_getc();
		if(temp != -1)
		{
			//string[i++] = temp;
			resp_ok[0] = resp_ok[1];
			resp_ok[1] = temp;
		}
		time_stop = HAL_GetTick();
		if((time_stop - time_start) > WAIT_TIMEOUT) return 1;
	}
	return 0;
}

/**
*/
int GSM_GetNetworkStatus(void)
{
	//AT+CREG?
	char resp_ok[2];
	uint32_t time_start, time_stop;
	int temp;
	int i = 0;
	
	time_start = HAL_GetTick();
	
	while(!strcmp(resp_ok, "OK"))
	{
		temp = UART_getc();
		if(temp != -1)
		{
			resp[i++] = temp;
			resp_ok[0] = resp_ok[1];
			resp_ok[1] = temp;
		}
		time_stop = HAL_GetTick();
		if((time_stop - time_start) > WAIT_TIMEOUT) return 1;
	}
	return 0;
}

int IsEnableGPRS(void)
{
	//AT+SAPBR=2,1
	//+SAPBR: 1,1,"xxx.xxx.xxx.xxx"
	//OK
	return 0;
}

void GSM_Init(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart)
{
	// Enable SIM900
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_Delay(2000);
	
	// Wait enable module
	
	HAL_UART_Transmit(user_uart, (uint8_t*)"Polus-IO test!", sizeof("Polus-IO test!"), 1000);
	//
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT", sizeof("AT"), 1000);
	GSM_WaitResp();
	
	HAL_Delay(2000);
	//
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", sizeof("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+SAPBR=3,1,\"APN\",\"internet.mts.ru\"", sizeof("AT+SAPBR=3,1,\"APN\",\"internet.mts.ru\""), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+SAPBR=3,1,\"USER\",\"mts\"", sizeof("AT+SAPBR=3,1,\"USER\",\"mts\""), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+SAPBR=3,1,\"PWD\",\"mts\"", sizeof("AT+SAPBR=3,1,\"PWD\",\"mts\""), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+SAPBR=1,1", sizeof("AT+SAPBR=1,1"), 1000);
	GSM_WaitResp();
	
	HAL_UART_Transmit(user_uart, (uint8_t*)"Checking for network...", sizeof("Checking for network..."), 1000);
	// wait registering
  while (GSM_GetNetworkStatus() != 1) 
  {
    HAL_Delay(500);  
  }

	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+CMEE=2", sizeof("AT+CMEE=2"), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(user_uart, (uint8_t*)"RSSI:", sizeof("RSSI:"), 1000);
	GSM_GetRSSI(gsm_uart);
	HAL_UART_Transmit(user_uart, (uint8_t*)gsm.rssi, sizeof(gsm.rssi), 1000);

	HAL_Delay(2000);
	HAL_UART_Transmit(user_uart, (uint8_t*)"Disabling GPRS", sizeof("Disabling GPRS"), 1000);

	//Disable the GPRS
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+SAPBR =0,1", sizeof("AT+SAPBR =0,1"), 1000);
	GSM_WaitResp();

	HAL_Delay(5000);
	HAL_UART_Transmit(user_uart, (uint8_t*)"Enabling GPRS", sizeof("Enabling GPRS"), 1000);
	//Enable the GPRS
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+SAPBR =1,1", sizeof("AT+SAPBR =1,1"), 1000);
	GSM_WaitResp();

	while(!IsEnableGPRS())
	{
		HAL_UART_Transmit(user_uart, (uint8_t*)"Failed to turn GPRS on", sizeof("Failed to turn GPRS on"), 1000);
		HAL_Delay(500);
	}
	//IMEI
	memset(gsm.imei,0,sizeof(gsm.imei));
	GSM_GetIMEI(gsm_uart);
	HAL_UART_Transmit(user_uart, (uint8_t*)"Connected to Cellular!", sizeof("Connected to Cellular!"), 1000);
	HAL_Delay(5000);
	
}

void Send2Site(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart)
{
	static int FAILURES;
	char* http_get;
//	  Watchdog.reset();
//    sensors.requestTemperatures();
//     float   Temp=sensors.getTempCByIndex(0);
//       Door=isDoorOpen();
//       Defrost=isDefrostOn();
//       Electricity=isElectricityOn();
//        Watchdog.reset();
//   Watchdog.reset();
  if(FAILURES==MAX_FAILURES) 
	{
		FAILURES=0;
//		fona.enableGPRS(false);
//		resetFunc(); 
	}
	
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+HTTPINIT", sizeof("AT+HTTPINIT"), 1000);
	GSM_WaitResp();
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+HTTPPARA=\"CID\",1", sizeof("AT+HTTPPARA=\"CID\",1"), 1000);
	GSM_WaitResp();
	
	http_get = malloc(sizeof(URL) + sizeof("&ts=-50") + sizeof("&tr=") + sizeof("Temp") + sizeof("&st=-25") + sizeof("&el=") + sizeof("Electricity") + sizeof("&dt=") + sizeof("Defrost") + sizeof("&door=") + sizeof("Door"));
		if(http_get == NULL) return;
		strcat(http_get, URL);
		strcat(http_get, "&ts=-50");
		strcat(http_get, "&tr=");
		strcat(http_get, "&st=-25");
		strcat(http_get, "&el=");
		strcat(http_get, "Electricity");
		strcat(http_get, "&dt=");
		strcat(http_get, "Defrost");
		strcat(http_get, "&door=");
		strcat(http_get, "Door");
		HAL_UART_Transmit(gsm_uart, (uint8_t*)http_get, sizeof(http_get), 1000);
		GSM_WaitResp();
		HAL_UART_Transmit(user_uart, (uint8_t*)http_get, sizeof(http_get), 1000);
	free(http_get);

	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+HTTPACTION=0", sizeof("AT+HTTPACTION=0"), 1000);
	GSM_WaitResp();

//  if(!fona.HTTP_action(0,buf1,buf2,15000)||strcmp("200",buf1)!=0)
//  {
//      Watchdog.reset();
//    FAILURES++; 
//  Serial.print(F("Failure++:"));Serial.println(FAILURES);
//  }
//  else
//  { Serial.println(F("Sucessfully send!"));}
// Watchdog.reset();
	
	HAL_UART_Transmit(gsm_uart, (uint8_t*)"AT+HTTPTERM", sizeof("AT+HTTPTERM"), 1000);
	GSM_WaitResp();

	HAL_Delay(500);
}
