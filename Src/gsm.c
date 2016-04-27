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

GSMTypeDef gsm;
char resp[50];

int GSM_WaitResp(void)
{
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

/**
*/
int GSM_GetIMEI(void)
{
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

/**
*/
int GSM_GetRSSI(char* rssi)
{
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
	GSM_GetRSSI(gsm.rssi);
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
	GSM_GetIMEI();
	HAL_UART_Transmit(user_uart, (uint8_t*)"Connected to Cellular!", sizeof("Connected to Cellular!"), 1000);
	HAL_Delay(5000);
	
}

void Send2Site(UART_HandleTypeDef *gsm_uart, UART_HandleTypeDef *user_uart)
{
//	  Watchdog.reset();
//    sensors.requestTemperatures();
//     float   Temp=sensors.getTempCByIndex(0);
//       Door=isDoorOpen();
//       Defrost=isDefrostOn();
//       Electricity=isElectricityOn();
//        Watchdog.reset();
//   Watchdog.reset();
//  if(FAILURES==MAX_FAILURES) 
//        {
//          FAILURES=0;
//          fona.enableGPRS(false);
//          resetFunc(); 
//        }
//         Watchdog.reset();
//  fona.HTTP_init();
//   Watchdog.reset();
//  fona.HTTP_para(F("CID"),"1");
//     Watchdog.reset();
//  fona.HTTP_para_start(F("URL"));
//  Watchdog.reset();
//  fona.print(F("http://minachevamir.myjino.ru/rest/add.php?imei="));
//  Serial.print(F("http://minachevamir.myjino.ru/rest/add.php?imei="));
//  Watchdog.reset();
//  fona.print(IMEI);
//  Serial.print(IMEI);
//  Watchdog.reset();
//  fona.print(F("&ts=-50")); 
//   Serial.print(F("&ts=-50"));
//  fona.print(F("&tr="));
//  Serial.print(F("&tr="));
//  fona.print(Temp);
//  Serial.print(Temp);
//  fona.print(F("&st=-25"));
//  Serial.print(F("&st=-25"));
//  Watchdog.reset();  
//  fona.print(F("&el="));
//  Serial.print(F("&el="));
//  fona.print(Electricity);
//  Serial.print(Electricity);
//  fona.print(F("&dt="));
//  Serial.print(F("&dt="));
//  fona.print(Defrost);
//  Serial.print(Defrost);
//  fona.print(F("&door="));
//  Serial.print(F("&door="));
//  fona.print(Door);
//  Serial.print(Door);
//  Watchdog.reset();
// 
// // Serial.println(F("&tempsupply=-18.4&tempreturn=-17.0&setpoint=-18.0&electricity=1&defrost=0&door=1"));

//  fona.HTTP_para_end();
//  Watchdog.reset();
//        
//// fona.HTTP_para(F("URL"),"http://minachevamir.myjino.ru/rest/add.php?imei=1");

//  if(!fona.HTTP_action(0,buf1,buf2,15000)||strcmp("200",buf1)!=0)
//  {
//      Watchdog.reset();
//    FAILURES++; 
//  Serial.print(F("Failure++:"));Serial.println(FAILURES);
//  }
//  else
//  { Serial.println(F("Sucessfully send!"));}
// Watchdog.reset();
//   fona.HTTP_term();
//      Watchdog.reset();
//   delay(500);
}
