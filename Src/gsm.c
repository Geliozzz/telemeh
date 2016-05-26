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
#define URL "\"http://minachevamir.myjino.ru/rest/add.php?imei="

#define MAX_FAILURES 3

GSMTypeDef gsm;
char resp[50];
const char *settingsForInternet[] = {"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r", 
								"AT+SAPBR=3,1,\"APN\",\"internet.beeline.ru\"\r",
								"AT+SAPBR=3,1,\"USER\",\"beeline\"\r",
								"AT+SAPBR=3,1,\"PWD\",\"beeline\"\r"};

int strequal(const char* str1,const char* str2)
{
	int len;
	len = strlen(str2);
	while(len--)
	{
		if(*str1 != *str2) return 1;
		str1++;
		str2++;
	}
	return 0;
}

int GSM_WaitResp(void)
{
	char resp_ok[2];
	char resp_error[5];
	int i;
	uint32_t time_start, time_stop;
	int temp;
	
	time_start = HAL_GetTick();
	
	//while(!(resp_ok[0] == 0x4F && resp_ok[1] == 0x4B))
	while(strequal(resp_ok, "OK"))
	{
		temp = UART_getc();
		if(temp != -1)
		{
			resp_ok[0] = resp_ok[1];
			resp_ok[1] = temp;
			for(i = 0; i < 5; i++)
			{
				resp_error[i] = resp_error[i + 1];
			}
			resp_error[4] = temp;
			if(!strequal(resp_error, "ERROR"))
			{
				return GSM_ERROR;
			}
		}
		time_stop = HAL_GetTick();
		if((time_stop - time_start) > WAIT_TIMEOUT) return GSM_TIMEOUT;
	}
	return GSM_OK;
}

/**
*/
int GSM_GetIMEI()
{
	uint32_t time_start, time_stop;
	int temp;
	int i,j = 0;
	char resp_ok[2];
	char resp_imei[30];
	
	time_start = HAL_GetTick();
	while(strequal(resp_ok, "OK\r" ))
	{
		temp = UART_getc();
		if(temp != -1)
		{
			for(i = 0; i < 29; i++)
			{
				resp_imei[i] = resp_imei[i + 1];
			}
			resp_imei[29] = temp; 
			resp_ok[0] = resp_ok[1];
			resp_ok[1] = temp;
		}
		time_stop = HAL_GetTick();
		if((time_stop - time_start) > WAIT_TIMEOUT) return GSM_TIMEOUT;
	}
	j = 0;
	for(i = 0; i < 30; i++)
	{
		if(resp_imei[i] >= 0x30 && resp_imei[i] <= 0x39)
		{
			gsm.imei[j++] = resp_imei[i];
		}
	}
	return GSM_OK;
}

/**
*/
int GSM_GetRSSI()
{
//	char string[20];
	char resp_ok[2];
	uint32_t time_start, time_stop;
	int temp;
//	int i = 0;
	
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
	
	while(!(resp_ok[0] == 0x4F && resp_ok[1] == 0x4B))
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

int GSM_GetAction(void)
{
	char resp_action[23];
	char resp_ok[2];
	int i,j;
	uint32_t time_start, time_stop;
	int temp;
	
	time_start = HAL_GetTick();
	
	while(strequal(resp_ok, "OK" ))
	{
		temp = UART_getc();
		if(temp != -1)
		{
			resp_ok[0] = resp_ok[1];
			resp_ok[1] = temp;
		}
		time_stop = HAL_GetTick();
		if((time_stop - time_start) > WAIT_TIMEOUT)
			return GSM_TIMEOUT;
	}
	HAL_Delay(2000);
	time_start = HAL_GetTick();
	i = 0;
	while(strequal(resp_action, "+HTTP" ))
	{
		temp = UART_getc();
		if(temp != -1)
		{
			for(i = 0; i < 23; i++)
			{
				resp_action[i] = resp_action[i + 1];
			}
			
			resp_action[22] = temp;
		}
		time_stop = HAL_GetTick();
		if((time_stop - time_start) > WAIT_TIMEOUT)
			return GSM_TIMEOUT;
	}
	for(i = 0; i < 30; i++)
	{
		if(resp_action[i] >= 0x30 && resp_action[i] <= 0x39)
		{
			gsm.action[j++] = resp_action[i];
		}
	}
	return GSM_OK;
}

int IsEnableGPRS(void)
{
	//AT+SAPBR=2,1
	//+SAPBR: 1,1,"xxx.xxx.xxx.xxx"
	//OK
	return 0;
}

void GSM_FailHandler(int fail)
{
	if (fail != GSM_OK) gsm.failtures++;
	return;
}

int GSM_SendCmd(UART_HandleTypeDef *gsm_uart, const char* str, int type)
{
	HAL_UART_Transmit(gsm_uart, (uint8_t*)str, strlen(str), 1000);
	switch (type)
	{
		case RESP_OK:
		{
			GSM_FailHandler(GSM_WaitResp());
		}
		break;
		case RESP_IMEI:
		{
			GSM_FailHandler(GSM_GetIMEI());
		}
		break;
		case RESP_RSSI:
		{
			GSM_GetRSSI();
		}
		break;
		case RESP_HTTPACTION:
		{
			GSM_FailHandler(GSM_GetAction());
		}
		break;
		default:
		{
			
		}
	}
	GSM_WaitResp();
	return 0;
}

int isDoorOpen(void)
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9);
}

int isDefrostOn(void)
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
}


void GSM_Init(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart)
{
	int i;
	// Enable SIM900
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_Delay(2000);
	
	// Wait enable module
	
	HAL_UART_Transmit(user_uart, (uint8_t*)"Polus-IO test!\n", sizeof("Polus-IO test!\n"), 1000);
	//
	for(i = 0; i < 5; i++)
	{
		GSM_SendCmd(gsm_uart, "AT\r", RESP_OK);
	}
	//
	GSM_SendCmd(gsm_uart, settingsForInternet[0], RESP_OK);
	GSM_SendCmd(gsm_uart, settingsForInternet[1], RESP_OK);
	GSM_SendCmd(gsm_uart, settingsForInternet[2], RESP_OK);
	GSM_SendCmd(gsm_uart, settingsForInternet[3], RESP_OK);
	GSM_SendCmd(gsm_uart, "AT+SAPBR=1,1\r", RESP_OK);


	
	HAL_UART_Transmit(user_uart, (uint8_t*)"Checking for network...", sizeof("Checking for network..."), 1000);
	// wait registering
  while (GSM_GetNetworkStatus() != 1) 
  {
    HAL_Delay(500);  
  }
	GSM_SendCmd(gsm_uart, "AT+CMEE=2\r", RESP_OK);

	HAL_UART_Transmit(user_uart, (uint8_t*)"RSSI:", sizeof("RSSI:"), 1000);
//	GSM_GetRSSI(gsm_uart);
//	GSM_SendCmd(gsm_uart, "AT+CSQ\r", RESP_RSSI);
	HAL_UART_Transmit(user_uart, (uint8_t*)gsm.rssi, sizeof(gsm.rssi), 1000);

	HAL_Delay(2000);
	HAL_UART_Transmit(user_uart, (uint8_t*)"Disabling GPRS", sizeof("Disabling GPRS"), 1000);

	//Disable the GPRS
	GSM_SendCmd(gsm_uart, "AT+SAPBR =0,1\r", RESP_OK);

	HAL_Delay(5000);
	HAL_UART_Transmit(user_uart, (uint8_t*)"Enabling GPRS", sizeof("Enabling GPRS"), 1000);
	//Enable the GPRS
	GSM_SendCmd(gsm_uart, "AT+SAPBR =1,1\r", RESP_OK);


	while(!IsEnableGPRS())
	{
		HAL_UART_Transmit(user_uart, (uint8_t*)"Failed to turn GPRS on", sizeof("Failed to turn GPRS on"), 1000);
		HAL_Delay(500);
		gsm.failtures++;
		break;
	}
	//IMEI
	//memset(gsm.imei,0,sizeof(gsm.imei));
	GSM_SendCmd(gsm_uart, "AT+GSN\r", RESP_IMEI);
	HAL_UART_Transmit(user_uart, (uint8_t*)"Connected to Cellular!", sizeof("Connected to Cellular!"), 1000);
	HAL_Delay(5000);
//	GSM_SendCmd(gsm_uart, "AT+HTTPINIT\r", RESP_OK);
//	GSM_SendCmd(gsm_uart, "AT+HTTPPARA=\"CID\",\"1\"\r", RESP_OK);
}

void Send2Site(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart)
{
	char* http_get;
	int size;
	char temp1_str[10];
	char temp2_str[10];
//	char  Door, Defrost;
	simple_float temp;

//	  Watchdog.reset();
	temp = ds18b20_GetTemp1();
	sprintf(temp1_str, "%d.%d2", temp.integer, temp.fractional);
	temp = ds18b20_GetTemp2();
	sprintf(temp2_str, "%d.%d2", temp.integer, temp.fractional);

	
//  Defrost=isDefrostOn();
//       Electricity=isElectricityOn();
//        Watchdog.reset();
//   Watchdog.reset();
  if(gsm.failtures >= MAX_FAILURES) 
	{
	//	gsm.failtures = 0;
//		fona.enableGPRS(false);
//		resetFunc(); 
	}
	
	GSM_SendCmd(gsm_uart, "AT+SAPBR=2,1\r", RESP_OK);
	GSM_SendCmd(gsm_uart, "AT+HTTPINIT\r", RESP_OK);
	GSM_SendCmd(gsm_uart, "AT+HTTPPARA=\"CID\",\"1\"\r", RESP_OK);
	

	size = sizeof("\"AT+HTTPPARA=\"URL\",") + sizeof(URL) + sizeof(gsm.imei) + sizeof("&ts=-50") + sizeof("&tr=") + 50 + sizeof("&st=-25")
										+ sizeof("&el=") + sizeof("20") + sizeof("&dt=") 
										+ sizeof("1") + sizeof("&door=") + sizeof("0\"\r");
	http_get = malloc(size);
		if(http_get == NULL) return;
		strcat(http_get, "\"AT+HTTPPARA=\"URL\",");
		strcat(http_get, URL);
		strcat(http_get, gsm.imei);
		strcat(http_get, "&ts=-50");
		strcat(http_get, "&tr=");
		strcat(http_get, temp1_str);
		strcat(http_get, "&st=-25");
		strcat(http_get, "&el=");
		strcat(http_get, "20");
		strcat(http_get, "&dt=");
		strcat(http_get, "1");
		strcat(http_get, "&door=");
		strcat(http_get, "0\"\r");
		GSM_SendCmd(gsm_uart, http_get, RESP_OK);
		HAL_UART_Transmit(user_uart, (uint8_t*)http_get, sizeof(*http_get), 1000);
	free(http_get);

	GSM_SendCmd(gsm_uart, "AT+HTTPACTION=0\r", RESP_HTTPACTION);
	HAL_Delay(3000);

//  if(!fona.HTTP_action(0,buf1,buf2,15000)||strcmp("200",buf1)!=0)
//  {
//      Watchdog.reset();
//    FAILURES++; 
//  Serial.print(F("Failure++:"));Serial.println(FAILURES);
//  }
//  else
//  { Serial.println(F("Sucessfully send!"));}
// Watchdog.reset();
	GSM_SendCmd(gsm_uart, "AT+HTTPTERM\r", RESP_OK);


	HAL_Delay(500);
}
