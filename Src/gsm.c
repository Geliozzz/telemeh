/****************************************************************
*  GSM.                                                         *
* ------------------------------------------------------------- *
*  АВТОР: Почепцов Олег Анатольевич	   												  *
* ------------------------------------------------------------- *
*****************************************************************/
#include "gsm.h"

#define WAIT_TIMEOUT 1000
#define URL "GET http://minachevamir.myjino.ru/rest/add.php?imei="
#define test_http "GET http://minachevamir.myjino.ru/rest/add.php?imei=863591022837136&ts=-50&tr=26.622&st=-25&el=20&dt=1&door=9\r\n"

#define MAX_FAILURES 10
// Stsndart delay
#define std_delay 1000
// Operator
//#define beeline

GSMTypeDef gsm;
char resp[50];
const char *settingsForInternet[] = {"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n", 
								"AT+SAPBR=3,1,\"APN\",\"internet.tele2.ru\"\r\n",
								"AT+SAPBR=3,1,\"USER\",\"\"\r\n",
								"AT+SAPBR=3,1,\"PWD\",\"\"\r\n"};

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
		if((time_stop - time_start) > WAIT_TIMEOUT) 
			return GSM_TIMEOUT;
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
	while(strequal(resp_ok, "OK" ))
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
	return 1;
}

void GSM_FailHandler(int fail)
{
	if (fail == GSM_TIMEOUT) gsm.failtures++;
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


void GSM_Init(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart, IWDG_HandleTypeDef *hiwdg)
{
	int i;
	// Enable 4V
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_Delay(std_delay);
	HAL_IWDG_Refresh(hiwdg);
	
	// Enable SIM900
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_Delay(std_delay * 2);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_Delay(std_delay * 2);
	HAL_IWDG_Refresh(hiwdg);
	
	// Wait enable module
	HAL_UART_Transmit(user_uart, (uint8_t*)"Polus-IO test!\n", sizeof("Polus-IO test!\n"), 1000);
	//
	for(i = 0; i < 1; i++)
	{
		GSM_SendCmd(gsm_uart, "AT\r\n", RESP_OK);
	}
	HAL_Delay(std_delay);
	GSM_SendCmd(gsm_uart, "AT+CREG? \r", RESP_OK);
	HAL_Delay(std_delay * 2);
	HAL_IWDG_Refresh(hiwdg);
	// Рвем соединение
	GSM_SendCmd(gsm_uart, "AT+CIPSHUT\r", RESP_OK);
	HAL_Delay(std_delay * 2);
	GSM_SendCmd(gsm_uart, "AT+CIPMUX=0\r", RESP_OK);
	HAL_Delay(std_delay);
	HAL_IWDG_Refresh(hiwdg);
	// Настраиваем интернет
	#ifdef beeline
		GSM_SendCmd(gsm_uart, "AT+CSTT=\"internet.beeline.ru\", \"beeline\", \"beeline\"\r", RESP_OK);
	#else
		GSM_SendCmd(gsm_uart, "AT+CSTT=\"internet.tele2.ru\", \"\", \"\"\r", RESP_OK);
	#endif
	
	HAL_Delay(std_delay);
	// Устанавливаем соединение
	GSM_SendCmd(gsm_uart, "AT+CIICR\r", RESP_OK);
	HAL_Delay(std_delay);
	HAL_IWDG_Refresh(hiwdg);

	HAL_UART_Transmit(user_uart, (uint8_t*)"Checking for network...", sizeof("Checking for network..."), 1000);
	// wait registering
  while (GSM_GetNetworkStatus() != 1) 
  {
    HAL_Delay(std_delay);  
  }
	HAL_IWDG_Refresh(hiwdg);
	// Выставляем глубину отображения ошибки
	GSM_SendCmd(gsm_uart, "AT+CMEE=2\r", RESP_OK);

	HAL_UART_Transmit(user_uart, (uint8_t*)"RSSI:", sizeof("RSSI:"), 1000);
	HAL_UART_Transmit(user_uart, (uint8_t*)gsm.rssi, sizeof(gsm.rssi), 1000);

	HAL_Delay(std_delay * 2);

	HAL_IWDG_Refresh(hiwdg);
	while(!IsEnableGPRS())
	{
		HAL_UART_Transmit(user_uart, (uint8_t*)"Failed to turn GPRS on", sizeof("Failed to turn GPRS on"), 1000);
		HAL_Delay(std_delay);
		gsm.failtures++;
		break;
	}
	HAL_IWDG_Refresh(hiwdg);
	GSM_SendCmd(gsm_uart, "AT+GSN\r", RESP_IMEI);
	HAL_UART_Transmit(user_uart, (uint8_t*)"Connected to Cellular!", sizeof("Connected to Cellular!"), 1000);
	HAL_Delay(std_delay * 5);
	HAL_IWDG_Refresh(hiwdg);
}

void Send2Site(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart, IWDG_HandleTypeDef *hiwdg)
{
	char* http_get;
	int size;
	char temp1_str[10];
	char temp2_str[10];
	char cmd_send[20];
	simple_float temp;

	HAL_IWDG_Refresh(hiwdg);
	temp = ds18b20_GetTemp1();
	if (temp.is_valid)
	{
		sprintf(temp1_str, "%d.%d2", temp.integer, temp.fractional);
	} 
	else sprintf(temp1_str, "-100");
	sprintf(temp1_str, "%d.%d2", temp.integer, temp.fractional);
	HAL_IWDG_Refresh(hiwdg);
	temp = ds18b20_GetTemp2();
	if (temp.is_valid)
	{
		sprintf(temp2_str, "%d.%d2", temp.integer, temp.fractional);
	} 
	else sprintf(temp2_str, "-100");
	
	// if imei is empty try get imei
	if (strlen(gsm.imei) < 15) GSM_SendCmd(gsm_uart, "AT+GSN\r", RESP_IMEI);
	HAL_IWDG_Refresh(hiwdg);
  if(gsm.failtures >= MAX_FAILURES) 
	{ 
		// Disable 4V 
		HAL_GPIO_WritePin(pm_GPIO_Port, pm_Pin, GPIO_PIN_RESET);
		HAL_Delay(std_delay);
		// Reset 
		HAL_NVIC_SystemReset();
	}

	GSM_SendCmd(gsm_uart, "AT+CIFSR\r", RESP_OK);
	HAL_Delay(std_delay);
	GSM_SendCmd(gsm_uart, "AT+CIPCLOSE\r", RESP_OK);
	HAL_Delay(std_delay);
	HAL_IWDG_Refresh(hiwdg);
	GSM_SendCmd(gsm_uart, "AT+CIPSTART=\"TCP\",\"minachevamir.myjino.ru\",80\r", RESP_OK);
	HAL_Delay(std_delay * 3);
	HAL_IWDG_Refresh(hiwdg);
	
	size = sizeof(URL) + sizeof(gsm.imei) + sizeof("&ts=") + sizeof("&tr=") + 80 + sizeof("&st=-25")
										+ sizeof("&el=") + sizeof("20") + sizeof("&dt=") 
										+ sizeof("1") + sizeof("&door=");
										
	http_get = malloc(size);
	if(http_get == NULL) return;
		memset(http_get, 0, size);
		strcat(http_get, URL);
		strcat(http_get, gsm.imei);
		strcat(http_get, "&ts=");
		strcat(http_get, temp1_str);						
		strcat(http_get, "&tr=");
		strcat(http_get, temp2_str);
		strcat(http_get, "&st=");
		strcat(http_get, "0");
		strcat(http_get, "&el=");
		if (isElectricityOn())strcat(http_get, "1");
		else strcat(http_get, "0");
		strcat(http_get, "&dt=");
		if (isDefrostOn())strcat(http_get, "1");
		else strcat(http_get, "0");
		strcat(http_get, "&door=");
		if (isDoorOpen())strcat(http_get, "1");
		else strcat(http_get, "0");
		strcat(http_get, "\r\n");

		HAL_IWDG_Refresh(hiwdg);
		sprintf(cmd_send, "AT+CIPSEND = %d\r", strlen(http_get));
		GSM_SendCmd(gsm_uart, cmd_send, RESP_OK);
		HAL_Delay(std_delay);
		HAL_IWDG_Refresh(hiwdg);
		GSM_SendCmd(gsm_uart, http_get, RESP_OK);		
		free(http_get);
		HAL_IWDG_Refresh(hiwdg);
		HAL_Delay(std_delay * 8);			
		HAL_IWDG_Refresh(hiwdg);
}

int isDefrostOn( )
{
	return	HAL_GPIO_ReadPin(Heater_GPIO_Port, Heater_Pin) > 0;
}
int isElectricityOn( )
{
	return	HAL_GPIO_ReadPin(Power_GPIO_Port, Power_Pin) > 0;
}
int isDoorOpen( )
{
	return	HAL_GPIO_ReadPin(Door_GPIO_Port, Door_Pin) > 0;
}
