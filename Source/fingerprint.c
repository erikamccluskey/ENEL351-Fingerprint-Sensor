/*
This is the library source file provided by Waveshare for the fingeprint sensor
You can find this code here: https://www.waveshare.com/wiki/UART_Fingerprint_Sensor_(C) under Demo code

File: fingerprint.c
Purpose: Functions for the functionalities of the fingerprint sensor 

Author: Waveshare.com, Modified by Erika McCluskey, Hayden Jin
*/

#include "fingerprint.h"
#include "stm32f1xx_hal.h"
			
uint8_t finger_TxBuf[9];			
uint8_t     Finger_SleepFlag;


/***************************************************************************
* @brief      Send a byte of data to the serial port
* @param      temp : Data to send
****************************************************************************/
HAL_StatusTypeDef  TxByte(uint8_t temp)
{	
	return   HAL_UART_Transmit(&huart1, &temp, 1 , 100);    
}

/***************************************************************************
* @brief      send a command, and wait for the response of module
* @param      Scnt: The number of bytes to send
	      Rcnt: expect the number of bytes response module
	      Delay_ms: wait timeout
* @return     ACK_SUCCESS: success
  	      other: see the macro definition
****************************************************************************/
uint8_t TxAndRxCmd(uint8_t Scnt, uint8_t Rcnt, uint16_t Delay_ms)
{
	uint8_t  i, j, CheckSum;
  uint32_t before_tick;        
  uint32_t after_tick;
  uint8_t   overflow_Flag = 0;
              
                       
	TxByte(CMD_HEAD);			 
	CheckSum = 0;
	for (i = 0; i < Scnt; i++)
	{
		TxByte(finger_TxBuf[i]);		 
		CheckSum ^= finger_TxBuf[i];
	}	
	TxByte(CheckSum);
	TxByte(CMD_TAIL);  
        
        
  Usart1_ReceiveStruct.RX_Size = 0;  // clear  RX_Size  before next receive
        
        // Receive time out: Delay_ms
  before_tick = HAL_GetTick();	 
  do
  {
    overflow_Flag = 0;
    after_tick = HAL_GetTick();	
    if(before_tick > after_tick)   //if overflow (go back to zero)
    {
      before_tick = HAL_GetTick();	  // get time_before again
      overflow_Flag = 1;
    }

  } while (((Usart1_ReceiveStruct.RX_Size < Rcnt) && (after_tick - before_tick < Delay_ms)) || (overflow_Flag == 1));
      
  if (Usart1_ReceiveStruct.RX_flag != 1)   return ACK_TIMEOUT;
	
  Usart1_ReceiveStruct.RX_flag = 0;	// clean flag
        
	if (Usart1_ReceiveStruct.RX_Size!= Rcnt)	return ACK_TIMEOUT;
	if (Usart1_ReceiveStruct.RX_pData[0] != CMD_HEAD) 	   return ACK_FAIL;
	if (Usart1_ReceiveStruct.RX_pData[Rcnt - 1] != CMD_TAIL)    return ACK_FAIL;
	if (Usart1_ReceiveStruct.RX_pData[1] != (finger_TxBuf[0]))  return ACK_FAIL;

	CheckSum = 0;
	
	for (j = 1; j < (Usart1_ReceiveStruct.RX_Size) - 1; j++) CheckSum ^= Usart1_ReceiveStruct.RX_pData[j];
	
	if (CheckSum != 0)   return ACK_FAIL; 	  

	return  ACK_SUCCESS;
}	 

/***************************************************************************
* @brief      Set Compare Level
* @param      temp: Compare Level,the default value is 5, can be set to 0-9, the bigger, the stricter
* @return     0xFF: error
  	      other: success, the value is compare level
****************************************************************************/
uint8_t SetcompareLevel(uint8_t temp)
{
	uint8_t m;
	
	finger_TxBuf[0] = CMD_COM_LEV;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = temp;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;	
	
	m = TxAndRxCmd(5, 8, 100);
		
	if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
	{
	    return Usart1_ReceiveStruct.RX_pData[3];
	}
	else
	{
	 	return 0xFF;
	}
}

/***************************************************************************
* @brief      Register fingerprint
* @return     ACK_SUCCESS: success
  	      other: see the macro definition
****************************************************************************/
uint8_t AddUser(void)
{
	uint8_t m;

	finger_TxBuf[0] = CMD_ADD_1;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = m +1;
	finger_TxBuf[3] = 3;
	finger_TxBuf[4] = 0;		
	m = TxAndRxCmd(5, 8, 5000);	
	if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
	{
		finger_TxBuf[0] = CMD_ADD_3;
		m = TxAndRxCmd(5, 8, 5000);
		if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
		{
			return ACK_SUCCESS;
		}
		else
		  return ACK_FAIL;
	}
	else
		return ACK_GO_OUT;
}

/***************************************************************************
* @brief      Clear fingerprints
* @return     ACK_SUCCESS:  success
  	      ACK_FAIL:     error
****************************************************************************/
uint8_t  ClearAllUser(void)
{
 	uint8_t m;
	
	finger_TxBuf[0] = CMD_DEL_ALL;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;
	
	m = TxAndRxCmd(5, 8, 500);
	
	if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
	{	    
		return ACK_SUCCESS;
	}
	else
	{
		return ACK_FAIL;
	}
}

/***************************************************************************
* @brief      Check if user ID is between 1 and 3
* @return     TRUE
  	      FALSE
****************************************************************************/
uint8_t IsMasterUser(uint8_t UserID)
{
    if ((UserID == 1) || (UserID == 2) || (UserID == 3)) return TRUE;
		else  return FALSE;
}	 

/***************************************************************************
* @brief      Fingerprint matching
* @return     ACK_SUCCESS: success
  	      other: see the macro definition
****************************************************************************/
uint8_t VerifyUser(void)
{
	uint8_t m;
	
	finger_TxBuf[0] = CMD_MATCH;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;
	
	m = TxAndRxCmd(5, 8, 5000);
	
	if ((m == ACK_SUCCESS) && (IsMasterUser(Usart1_ReceiveStruct.RX_pData[4]) == TRUE))
	{	
		 return ACK_SUCCESS;
	}
	else if(Usart1_ReceiveStruct.RX_pData[4] == ACK_NO_USER)
	{
		return ACK_NO_USER;
	}
	else if(Usart1_ReceiveStruct.RX_pData[4] == ACK_TIMEOUT)
	{
		return ACK_TIMEOUT;
	}
	else{
		return ACK_FAIL;
	}
}

