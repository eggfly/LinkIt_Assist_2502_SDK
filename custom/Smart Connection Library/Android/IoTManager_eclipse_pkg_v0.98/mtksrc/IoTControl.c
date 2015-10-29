#include "IoTControl.h"
#include "crypt_aes.h"
#include <android/log.h>
#ifdef HWTEST_LOG_TAG
#undef HWTEST_LOG_TAG
#define HWTEST_LOG_TAG "IoTControl"
#endif
#define CTRL_DATA_ENCRYPT 1
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "IoTContol.c", __VA_ARGS__)
CtrlAdpter g_CtrlAdpter;

int ReceiveMessage(ClientInfo *pClient,
						BYTE *Buf,
						UINT_32 *iLen,
						BYTE Type,
						BYTE SubType
						);

/*
 * Description : Check the message contain Status
 * Param IN
 * 		@Buffer     : Buffer of Mesage
 * Return
 * 		0: success
 * 		-1: error
 */

int CheckSatus(void *Buffer)
{
	DataHeader *pDataHeader;
	UINT_32 DataLen = 0;
	Status *pStatus = NULL;
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
		return -1;
	}
	
	pDataHeader = (DataHeader *)((UINT_8 *)Buffer + sizeof(IoTCtrlProtocolHeader));
	
	if (STATUS != pDataHeader->Type)
	{
		Log_Printf("Drop the contain status message: pDataHeader->Type [%#x] not matched [%#x]\n", 
					pDataHeader->Type, STATUS);
		return -1;
	}
	
	pStatus = (Status*)((UINT_8 *)pDataHeader + sizeof(pDataHeader));
	
	if (0 == pStatus->StatusCode)
	{
		return 0;
	}
	
	return -1;
}

int MessageSanity(ClientInfo *pClient, void *Buffer, UINT_32 iLen)
{
	BYTE Type = 0;
	BYTE SubType = 0;
	IoTCtrlProtocolHeader *pMessageHeader;
	UINT_16 RecvSequence = 0;
	UINT_8 BroadMACAddress[MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
		return -1;
	}
	
	pMessageHeader = (IoTCtrlProtocolHeader *)Buffer;
	
	//1. check the magic number of protocol
	if (IOT_MAGIC_NUMBER != pMessageHeader->Magic)
	{
		Log_Printf("Drop the message: Magic number [%#x] not matched [%#x]\n", 
					pMessageHeader->Magic, IOT_MAGIC_NUMBER);
		return -1;
	}
	
	//2. Filter the message of g_SequenceNum not matched
	RecvSequence = pMessageHeader->Sequence;
	if (RecvSequence != pClient->SequenceNum)
	{
		Log_Printf("Drop the message: RecvSequence number [%#x] not matched [%#x]\n", 
					RecvSequence, pClient->SequenceNum);
		return -1;
	}
	
	//3. Filter the message of local end
	if (!memcmp(g_CtrlAdpter.OwnMACAddress, pMessageHeader->SendMAC, MAC_ADDR_LEN))
	{
		Log_Printf("Drop the message: SendMAC is Local MAC\n");
		return -1;
	}
	
	//4. check the recevied ID is own or broadcast mac address
	if (memcmp(g_CtrlAdpter.OwnMACAddress, pMessageHeader->ReceiveMAC, MAC_ADDR_LEN) &&
		memcmp(BroadMACAddress, pMessageHeader->ReceiveMAC, MAC_ADDR_LEN))
	{
		Log_Printf("Drop the message: ReceiveMAC not matched own MAC or FF:FF:FF:FF:FF:FF\n");
		return -1;
	}
	
	return 0;
}

/*
 * 
 * return :  ClientID matched ClientInfo
 * 
 */
ClientInfo *GetClientInfoByClientID(UINT_32 ClientID)
{
	ClientInfo *pClient = NULL;
	
	pClient = g_CtrlAdpter.ClientInfoList;
	
	while (NULL != pClient)
	{
		if (pClient->ClientID == ClientID)
		{
			Log_Printf("ClientID[%d] matched MAC Addr has found", ClientID);
			return pClient;
		}
		
		pClient = pClient->Next;
	}
	
	return pClient;
}

int ClearClientByClientID(int ClientID)
{
	ClientInfo *pClient = NULL;
	ClientInfo *pTargetClient = NULL;
	
	pClient = g_CtrlAdpter.ClientInfoList;
	Log_Printf("ClientID[%d] ClearClientByClientID", ClientID);
	if (g_CtrlAdpter.ClientInfoList->ClientID == ClientID)
	{
		g_CtrlAdpter.ClientInfoList = g_CtrlAdpter.ClientInfoList->Next;
		free((void*)pClient);
		return 0;
	}
	
	while (NULL != pClient->Next)
	{
		if (pClient->Next->ClientID == ClientID)
		{
			pTargetClient = pClient->Next;
			pClient->Next = pTargetClient->Next;
			free((void *)pTargetClient);

			return 0;
		}
		
		pClient = pClient->Next;
	}
	
	return -1;
}

int CtrlClientOffline(UINT_32 ClientID)
{
	ClientInfo *pClient = NULL;
	UINT_32 PackageLen = 0;
	int iRet = 0;
	char *SendBuf = NULL;
	
	SendBuf = g_CtrlAdpter.SendBuffer;
	pClient = GetClientInfoByClientID(ClientID);
	if (NULL == pClient)
	{
		Log_Printf("ClientID[%d] matched MAC Addr not founded", ClientID);
		return -1;
	}
	pClient->SequenceNum ++;

	memset(SendBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	PackageLen = ControlClientOfflineRequestBuild(SendBuf, pClient->ClientMACAddr, 
		g_CtrlAdpter.OwnMACAddress, pClient->SequenceNum, pClient->SessionID);
						
	iRet = SendMessage(pClient, SendBuf, PackageLen);
	
	ClearClientByClientID(ClientID);
	
	return 0;
}

/*************************************************
 *
 *			IoT Control API of IoTControl.so
 *
 *************************************************/

/*
 * Description : Set GPIO
 * Param IN
 * 		@ClientID     : The client of message send to 
 * 		@GPIOList    : GPIOList bitMAP. 1 means GPIO is available, 0 means not available
 *           @GPIOValue : GPIOValue bitMAP. only which bit set to 1 in GPIOList bitMAP are available,
 *					 0 means low level, 1 means high level
 * Return
 * 		0: success
 * 		-1: error
 */

int SetGPIO(UINT_32 ClientID, UINT_32 GPIOList, UINT_32 GPIOValue)
{
	ClientInfo *pClient = NULL;
	UINT_32 PackageLen = 0;
	int iRet = 0;
	int iLen = 0;
	char *SendBuf = NULL;
	char *RecvBuf = NULL;
	
	SendBuf = g_CtrlAdpter.SendBuffer;
	RecvBuf = g_CtrlAdpter.RecvBuffer;
	pClient = GetClientInfoByClientID(ClientID);
	if (NULL == pClient)
	{
		Log_Printf("ClientID[%d] matched MAC Addr not founded", ClientID);
		return -1;
	}
	pClient->SequenceNum ++;
	memset(SendBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	PackageLen = SetGPIORequestBuild(SendBuf, pClient->ClientMACAddr, 
			g_CtrlAdpter.OwnMACAddress, pClient->SequenceNum, 
			pClient->SessionID, GPIOList, GPIOValue);
						
	SendMessage(pClient, SendBuf, PackageLen);
	
	memset(RecvBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	iRet = ReceiveMessage(pClient, RecvBuf, &iLen, FUNCTION, GPIO_SET_RESPONSE);
	if (-1 == iRet)
	{
		Log_Printf("Message receive timeout");
		return -1;
	}
	if (0 == MessageSanity(pClient, RecvBuf, iLen))
	{
		iRet = CheckSatus(RecvBuf);
	}
	return iRet;
}

/*
 * Description : Get GPIO
 * Param 
 * 		@ClientID     : The client of message send to 
 * 		@*GPIOList    : Address of GPIOList bitMAP. 1 means GPIO is available, 0 means not available
 *           @*GPIOValue : Address of GPIOValue bitMAP. Only which bit set to 1 in GPIOList bitMAP are available,
 *					 0 means low level, 1 means high level
 * Return
 * 		0: success
 * 		-1: error
 */

int GetGPIO(UINT_32 ClientID, UINT_32 *GPIOList, UINT_32 *GPIOValue)
{
	ClientInfo *pClient = NULL;
	UINT_32 PackageLen = 0;
	UINT_32 iLen;
	UINT_32 iRet = 0;
	IoTCtrlProtocolHeader *pMessageHeader = NULL;
	DataHeader *pDataHead = NULL;
	GPIO_Information *pGPIO_InfoData = NULL;
	
	BYTE Type = 0;
	BYTE SubType = 0;
	char *SendBuf = NULL;
	char *RecvBuf = NULL;
	
	SendBuf = g_CtrlAdpter.SendBuffer;
	RecvBuf = g_CtrlAdpter.RecvBuffer;
	pClient = GetClientInfoByClientID(ClientID);
	if (NULL == pClient)
	{
		Log_Printf("ClientID[%d] matched MAC Addr not founded", ClientID);
		return -1;
	}
	pClient->SequenceNum ++;
	memset(SendBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	
	PackageLen = GetGPIORequestBuild(SendBuf, pClient->ClientMACAddr, 
		g_CtrlAdpter.OwnMACAddress, pClient->SequenceNum, pClient->SessionID);
	//send GetGPIO request message
	SendMessage(pClient, SendBuf, PackageLen);
	
	//wait for GetGPIO response message
	memset(RecvBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	iRet = ReceiveMessage(pClient, RecvBuf, &iLen, FUNCTION, GPIO_GET_RESPONSE);
	if (-1 == iRet)
	{
		Log_Printf("Message receive timeout");
		return -1;
	}
	//process GPIO_GET response message
	if (0 == MessageSanity(pClient, RecvBuf, iLen))
	{
		pMessageHeader = (IoTCtrlProtocolHeader *)RecvBuf;
		Type = pMessageHeader->Type;
		SubType = pMessageHeader->SubType;
		if ((FUNCTION != Type) || (GPIO_GET_RESPONSE != SubType))
		{
			Log_Printf("Drop the message: Not GPIO_GET response");
			return -1;			
		}

		pDataHead = (DataHeader *)((UINT_8 *)RecvBuf + sizeof(IoTCtrlProtocolHeader));
		if (GPIO_INFORMATION != pDataHead->Type)
		{
			Log_Printf("Drop the GPIO_GET response message: data type is not GPIO_INFORMATION");
			return -1;
		}

		pGPIO_InfoData = (GPIO_Information *)((UINT_8 *)pDataHead + sizeof(DataHeader));

		Log_Printf("pGPIO_InfoData GPIO List = %d", pGPIO_InfoData->GPIO_List);
		Log_Printf("pGPIO_InfoData GPIO Value = %d", pGPIO_InfoData->GPIO_Value);

		*GPIOList = pGPIO_InfoData->GPIO_List;
		*GPIOValue = pGPIO_InfoData->GPIO_Value;

		Log_Printf("GPIO List = %d", *GPIOList);
		Log_Printf("GPIO Value = %d", *GPIOValue);
		
	}
	
	return 0;
}

int SetUART(UINT_32 ClientID, const char *pData, int DataLen)
{
	ClientInfo *pClient = NULL;
	UINT_32 PackageLen = 0;
	char *SendBuf = NULL;
//	char *RecvBuf = NULL;
	
	SendBuf = g_CtrlAdpter.SendBuffer;
//	RecvBuf = g_CtrlAdpter.RecvBuffer;
	pClient = GetClientInfoByClientID(ClientID);
	if (NULL == pClient)
	{
		Log_Printf("ClientID[%d] matched MAC Addr not founded", ClientID);
		return -1;
	}
	
	pClient->SequenceNum ++;
	memset(SendBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	PackageLen = SetUARTRequestBuild(SendBuf, pClient->ClientMACAddr,
			g_CtrlAdpter.OwnMACAddress, pClient->SequenceNum, 
			pClient->SessionID, pData, DataLen);
	
	Log_Printf("Uart Len = %d", DataLen);					
	SendMessage(pClient, SendBuf, PackageLen);
	
	return 0;
}

int GetUART(UINT_32 ClientID, char *pData, int *DataLen)
{
	ClientInfo *pClient = NULL;
	UINT_32 PackageLen = 0;
	IoTCtrlProtocolHeader *pMessageHeader = NULL;
	DataHeader *pDataHead = NULL;
	UART_Information *pUartInforData = NULL;
	UINT_32 UartDataLen = 0;
	int iRet = 0;
	int iLen = 0;
	BYTE Type = 0;
	BYTE SubType = 0;
	char *SendBuf = NULL;
	char *RecvBuf = NULL;
	
	SendBuf = g_CtrlAdpter.SendBuffer;
	RecvBuf = g_CtrlAdpter.RecvBuffer;	
	
	pClient = GetClientInfoByClientID(ClientID);
	if (NULL == pClient)
	{
		Log_Printf("ClientID[%d] matched MAC Addr not founded", ClientID);
		return -1;
	}
	
	pClient->SequenceNum ++;
	memset(SendBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	PackageLen = GetUARTRequestBuild(SendBuf, pClient->ClientMACAddr,
				g_CtrlAdpter.OwnMACAddress, pClient->SequenceNum, pClient->SessionID);
						
	SendMessage(pClient, SendBuf, PackageLen);
	
	//wait for GetUART response message
	memset(RecvBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	iRet = ReceiveMessage(pClient, RecvBuf, &iLen, FUNCTION, UART_GET_RESPONSE);
	if (-1 == iRet)
	{
		Log_Printf("Message receive timeout");
		return -1;
	}
	//process GPIO_GET response message
	if (0 == MessageSanity(pClient, RecvBuf, iLen))
	{
		pMessageHeader = (IoTCtrlProtocolHeader *)RecvBuf;
		Type = pMessageHeader->Type;
		SubType = pMessageHeader->SubType;
		if ((FUNCTION != Type) || (UART_GET_RESPONSE != SubType))
		{
			Log_Printf("Drop the message: Not GPIO_GET response");
			return -1;			
		}

		pDataHead = (DataHeader *)((UINT_8 *)RecvBuf + sizeof(IoTCtrlProtocolHeader));
		if (UART_INFORMATION != pDataHead->Type)
		{
			Log_Printf("Drop the GPIO_GET response message: data type is not GPIO_INFORMATION");
			return -1;
		}

		pUartInforData = (UART_Information *)((UINT_8 *)pDataHead + sizeof(DataHeader));
		UartDataLen = pDataHead->Length;
		memcpy(pData, pUartInforData->Data, UartDataLen);
		*DataLen = UartDataLen;
	}
	return 0;
}

int SetPWM(UINT_32 ClientID, UINT_16 Red, UINT_16 Green, UINT_16 Blue)
{
	UINT_16 PWMDataLen = 6;
	ClientInfo *pClient = NULL;
	UINT_32 PackageLen = 0;
	UINT_16 PWMInfo[3] = {0};
	IoTCtrlProtocolHeader *pMessageHeader = NULL;
	DataHeader *pDataHead = NULL;
	PWM_Information *pPWMInforData = NULL;

	char *SendBuf = NULL;
	char *RecvBuf = NULL;
	int iRet = 0;
	int iLen = 0;
	BYTE Type = 0;
	BYTE SubType = 0;
	SendBuf = g_CtrlAdpter.SendBuffer;
	RecvBuf = g_CtrlAdpter.RecvBuffer;
	
	pClient = GetClientInfoByClientID(ClientID);
	if (NULL == pClient)
	{
		Log_Printf("ClientID[%d] matched MAC Addr not founded", ClientID);
		return -1;
	}
	pClient->SequenceNum ++;
	memset(SendBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	
	PWMInfo[0] = Red;
	PWMInfo[1] = Green;
	PWMInfo[2] = Blue;
	
	PackageLen = SetPWMRequestBuild(SendBuf, pClient->ClientMACAddr, 
		g_CtrlAdpter.OwnMACAddress, pClient->SequenceNum, 
		pClient->SessionID, (UINT_8 *)PWMInfo, PWMDataLen);
						
	SendMessage(pClient, SendBuf, PackageLen);
	//wait for SetPWM response message
		memset(RecvBuf, 0, PACKAGE_BUFFER_MAX_LEN);
		iRet = ReceiveMessage(pClient, RecvBuf, &iLen, FUNCTION, PWM_SET_RESPONSE);
		if (-1 == iRet)
		{
			Log_Printf("Message receive timeout");
			return -1;
		}

		//process GPIO_SET response message
			if (0 == MessageSanity(pClient, RecvBuf, iLen))
			{
				pMessageHeader = (IoTCtrlProtocolHeader *)RecvBuf;
				Type = pMessageHeader->Type;
				SubType = pMessageHeader->SubType;
				if ((FUNCTION != Type) || (PWM_SET_RESPONSE != SubType))
				{
					Log_Printf("Drop the message: Not PWM_SET response");
					return -1;
				}

				pDataHead = (DataHeader *)((UINT_8 *)RecvBuf + sizeof(IoTCtrlProtocolHeader));
				if (STATUS != pDataHead->Type)
				{
					Log_Printf("Drop the GPIO_SET response message: data type is not PWM_INFORMATION");
					return -1;
				}


			}

	return 0;
}

int GetPWM(UINT_32 ClientID, UINT_16 *Red, UINT_16 *Green, UINT_16 *Blue)
{
	ClientInfo *pClient = NULL;
	UINT_32 PackageLen = 0;
	IoTCtrlProtocolHeader *pMessageHeader = NULL;
	DataHeader *pDataHead = NULL;
	PWM_Information *pPWMInforData = NULL;
	UINT_32 UartDataLen = 0;
	int iRet = 0;
	int iLen = 0;
	BYTE Type = 0;
	BYTE SubType = 0;
	
	char *SendBuf = NULL;
	char *RecvBuf = NULL;
	
	SendBuf = g_CtrlAdpter.SendBuffer;
	RecvBuf = g_CtrlAdpter.RecvBuffer;	
	
	pClient = GetClientInfoByClientID(ClientID);
	if (NULL == pClient)
	{
		Log_Printf("ClientID[%d] matched MAC Addr not founded", ClientID);
		return -1;
	}
	pClient->SequenceNum ++;
	memset(SendBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	PackageLen = GetPWMRequestBuild(SendBuf, pClient->ClientMACAddr, 
			g_CtrlAdpter.OwnMACAddress, pClient->SequenceNum, pClient->SessionID);
						
	SendMessage(pClient, SendBuf, PackageLen);
	
	//wait for GetPWM response message
	memset(RecvBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	iRet = ReceiveMessage(pClient, RecvBuf, &iLen, FUNCTION, PWM_GET_RESPONSE);
	if (-1 == iRet)
	{
		Log_Printf("Message receive timeout");
		return -1;
	}
	//process GPIO_GET response message
	if (0 == MessageSanity(pClient, RecvBuf, iLen))
	{
		pMessageHeader = (IoTCtrlProtocolHeader *)RecvBuf;
		Type = pMessageHeader->Type;
		SubType = pMessageHeader->SubType;
		if ((FUNCTION != Type) || (PWM_GET_RESPONSE != SubType))
		{
			Log_Printf("Drop the message: Not PWM_GET response");
			return -1;			
		}

		pDataHead = (DataHeader *)((UINT_8 *)RecvBuf + sizeof(IoTCtrlProtocolHeader));
		if (PWM_INFORMATION != pDataHead->Type)
		{
			Log_Printf("Drop the GPIO_GET response message: data type is not PWM_INFORMATION");
			return -1;
		}

		pPWMInforData = (PWM_Information *)((UINT_8 *)pDataHead + sizeof(DataHeader));
		*Red = pPWMInforData->RedLevel;
		*Green = pPWMInforData->GreenLevel;
		*Blue = pPWMInforData->BlueLevel;
		
	}
	
	return 0;

}

int PasswordtoSessionID(const char *pPassword)
{
	int iLen = 0;
	int i = 0;
	int iSessionID = 0;

	if (NULL == pPassword)
	{
		return DEFAULT_SESSION_ID;
	}
	
	iLen = strlen(pPassword);
	if (iLen < 4)
	{
		return DEFAULT_SESSION_ID;
	}
	memcpy(&iSessionID, pPassword, 4);
	Log_Printf("Set control password: [0x%x]\n", iSessionID);

	return iSessionID;
}

int InitCtrlPassword(UINT_32 iSessionID)
{
	g_CtrlAdpter.SessionID= iSessionID;

	Log_Printf("Init control password: [0x%x]\n", iSessionID);
	
	return 0;
}

int SetDataEncrypt(int iDataEncrypt)
{
	g_CtrlAdpter.DataEncrpty = iDataEncrypt;
	Log_Printf("DataEncrpty = %d\n", g_CtrlAdpter.DataEncrpty);

	return 0;
}


/*Need re-design */
int SetCtrlPassword(UINT_32 iSessionID)
{
	ClientInfo *pClient = NULL;
	pClient = g_CtrlAdpter.ClientInfoList;

	while (NULL != pClient)
	{
		if (SetEachClientCtrlPassword(pClient, iSessionID) < 0)
		{
			return -1;
		}
		pClient = pClient->Next;
	}

	g_CtrlAdpter.SessionID = iSessionID;

	return 0;
}

int SetEachClientCtrlPassword(ClientInfo *pClient, UINT_32 iSessionID)
{
	UINT_32 PackageLen = 0;
	UINT_32 iLen;
	UINT_32 iRet = 0;
	IoTCtrlProtocolHeader *pMessageHeader = NULL;
	DataHeader *pDataHead = NULL;
	Status *pStatus = NULL;
	
	BYTE Type = 0;
	BYTE SubType = 0;
	char *SendBuf = NULL;
	char *RecvBuf = NULL;
	
	SendBuf = g_CtrlAdpter.SendBuffer;
	RecvBuf = g_CtrlAdpter.RecvBuffer;	
	
//	pClient = GetClientInfoByClientID(ClientID);
	//only set one client
	
	pClient->SequenceNum ++;
	memset(SendBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	PackageLen = ControlPasswordSetRequestBuild(SendBuf, pClient->ClientMACAddr, 
		g_CtrlAdpter.OwnMACAddress, pClient->SequenceNum, 
		pClient->SessionID, (UINT_8 *)&iSessionID, 4);
	
	//send Set ctrl password request message
	SendMessage(pClient, SendBuf, PackageLen);
	
	//wait for Set ctrl password response message
	memset(RecvBuf, 0, PACKAGE_BUFFER_MAX_LEN);
	iRet = ReceiveMessage(pClient, RecvBuf, &iLen, MANAGEMENT,
						CONTROL_PASSWORD_SET_RESPONSE);
	if (-1 == iRet)
	{
		Log_Printf("Message receive timeout");
		return -1;
	}
	//process Set ctrl password response message
	if (0 == MessageSanity(pClient, RecvBuf, iLen))
	{
		pMessageHeader = (IoTCtrlProtocolHeader *)RecvBuf;
		Type = pMessageHeader->Type;
		SubType = pMessageHeader->SubType;
		if ((MANAGEMENT != Type) || (CONTROL_PASSWORD_SET_RESPONSE != SubType))
		{
			Log_Printf("Drop the message: Not  Set_ctrl_password response");
			return -1;			
		}

		pDataHead = (DataHeader *)((UINT_8 *)RecvBuf + sizeof(IoTCtrlProtocolHeader));
		if (STATUS != pDataHead->Type)
		{
			Log_Printf("Drop the Set_ctrl_password response message");
			return -1;
		}

		pStatus = (Status *)((UINT_8 *)pDataHead + sizeof(DataHeader));
		if (0 != pStatus->StatusCode)
		{
			Log_Printf("Control password set Error!");
			return -1;
		}
		
		Log_Printf("Send Control password confirm message.");
		//send Set ctrl password confirm message
		pClient->SequenceNum ++;
		memset(SendBuf, 0, PACKAGE_BUFFER_MAX_LEN);
		PackageLen = ControlPasswordSetConfirmBuild(SendBuf, pClient->ClientMACAddr, 
				g_CtrlAdpter.OwnMACAddress, pClient->SequenceNum, pClient->SessionID);	
		SendMessage(pClient, SendBuf, PackageLen);
		
		pClient->SessionID = iSessionID;

		Log_Printf("Control password [0x%x] is set success.", iSessionID);
	}
	
	return 0;
}

#if 0
/*New thread*/
void *QueryThread(void *arg)
{
	int iRet = 0;
	int iLen = 0;
	
	while (g_QueryFlag)
	{
		memset(g_RecvBuffer, 0, PACKAGE_BUFFER_MAX_LEN);
		iRet = ReceiveMessage(g_RecvBuffer, &iLen, MANAGEMENT, QUERY_CAPAB_RESPONSE);
		iRet = MessageSanity(g_RecvBuffer, iLen);
	}
	
	pthread_exit((void *)1);
	
	return NULL;
}
#endif

/*
 * Description : Check Client MAC Address has created on ClientList
 * 
 * @ClientMACAddress : MAC address of Client
 * 
 * return : TRUE has found
 *             FALSE has not found 
 */
ClientInfo * ClientMatched(UINT_8 *pIPAddr)
{
	LOGD("ClientMatched Function");
	ClientInfo *pClient = NULL;
	pClient = g_CtrlAdpter.ClientInfoList;
	
	while (NULL != pClient)
	{
		if (!memcmp(pClient->ClientIPAddr, pIPAddr, strlen(pIPAddr)))
		{
			return pClient;
		}
		LOGD("pClient->ClientIPAddr = [%s]\n", pClient->ClientIPAddr);
		pClient = pClient->Next;
	}
	
	return NULL;
}

ClientInfo *AllocClient()
{
	ClientInfo *pClient = NULL;
	
	pClient = (ClientInfo *)malloc(sizeof(ClientInfo));
	memset(pClient, 0, sizeof(ClientInfo));
	
	return pClient;
}

int DeallocClient(ClientInfo *pDelClient)
{
	ClientInfo *pClient = NULL;
	
	if (NULL == pDelClient)
	{
		return -1;
	}

	if (pDelClient == g_CtrlAdpter.ClientInfoList)
	{
		g_CtrlAdpter.ClientInfoList = g_CtrlAdpter.ClientInfoList->Next;
		free(pDelClient);
		return 0;
	}
	
	pClient = g_CtrlAdpter.ClientInfoList;
	while (pClient != NULL)
	{
		if (pDelClient == pClient->Next)
		{
			pClient->Next = pDelClient->Next;
			free(pDelClient);
			return 0;
		}
		pClient = pClient->Next;
	}

	return -1;
}

int GetClientCapability(ClientInfo *pNewClient)
{
	int iRet = 0;
	int iLen = 0;
	UINT_8 BroadMACAddress[MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	IoTCtrlProtocolHeader *pMessageHeader;
	DataHeader *pDataHeader;
	UINT_16 RecvSequence = 0;
	UINT_32 DataLen = 0;
	ClientCapability *pClientCap = NULL;
	char *RecvBuffer = NULL;
	UINT_8 DestAddress[MAC_ADDR_LEN] ={0};
	
	pNewClient->SequenceNum ++;
	RecvBuffer = g_CtrlAdpter.RecvBuffer;
	Log_Printf("GetClientCapability");
	if (NULL == pNewClient)
	{
		Log_Printf("NULL == pNewClient");
		return -1;
	}
	memset(g_CtrlAdpter.SendBuffer, 0, PACKAGE_BUFFER_MAX_LEN);

	if (g_CtrlAdpter.NetworkType == NETWORK_WLAN)
	{
		memcpy(DestAddress, pNewClient->ClientMACAddr, MAC_ADDR_LEN);
	}
	else
	{
		memcpy(DestAddress, BroadMACAddress, MAC_ADDR_LEN);
	}
	
	iLen = QueryClientCapabilityRequestBuild((void *)g_CtrlAdpter.SendBuffer, 
						DestAddress, g_CtrlAdpter.OwnMACAddress, 
						pNewClient->SequenceNum, pNewClient->SessionID);
	
	SendMessage(pNewClient,
				g_CtrlAdpter.SendBuffer,
				iLen);
#if 1	
	memset(RecvBuffer, 0, PACKAGE_BUFFER_MAX_LEN);
	iRet = ReceiveMessage(pNewClient,
						  RecvBuffer, 
						  &iLen, 
						  MANAGEMENT,
						  QUERY_CAPAB_RESPONSE);
	if (-1 == iRet)
	{
		Log_Printf("Message receive timeout");
		return -1;
	}

	pMessageHeader = (IoTCtrlProtocolHeader *)RecvBuffer;
	pDataHeader = (DataHeader *)((UINT_8 *)RecvBuffer + sizeof(IoTCtrlProtocolHeader));
	
	Log_Printf("Process Query capablity response message.\n");
	
	RecvSequence = pMessageHeader->Sequence;
	if (RecvSequence != pNewClient->SequenceNum)
	{
		Log_Printf("Drop the QueryCapabResponse message: RecvSequence number [%#x] not matched [%#x]\n", 
					RecvSequence, pNewClient->SequenceNum);
		return -1;
	}
	
	if (CLIENT_CAPABILITY != pDataHeader->Type)
	{
		Log_Printf("Drop the QueryCapabResponse message: pDataHeader->Type [%#x] not matched [%#x]\n", 
					pDataHeader->Type, CLIENT_CAPABILITY);
		return -1;
	}
	
	if (!memcmp(BroadMACAddress, pMessageHeader->SendMAC, MAC_ADDR_LEN))
	{
		Log_Printf("Drop the QueryCapabResponse message: SendMAC not allowed\n");
		return -1;
	}
	
	DataLen = iLen - sizeof(IoTCtrlProtocolHeader) - sizeof(pDataHeader);
#if 0
	if (pDataHeader->Length != DataLen)
	{
		Log_Printf("Drop the QueryCapabResponse message: pDataHeader->Length [%#x] not matched [%#x]\n", 
					pDataHeader->Length, DataLen);
		return -1;
	}
#endif
	/*save client capability*/
	pClientCap = (ClientCapability *)((UINT_8 *)RecvBuffer + 
		sizeof(IoTCtrlProtocolHeader) + sizeof(pDataHeader));

	memcpy(pNewClient->ClientMACAddr, pMessageHeader->SendMAC, MAC_ADDR_LEN);
	
	Log_Printf("query response , ClientMAC:");

	int i = 0;
	for (i = 0; i < MAC_ADDR_LEN; i ++)
	Log_Printf("[%#x]", pNewClient->ClientMACAddr[i] & 0xFF);
	
	pNewClient->ClientCapab.VendorNameLen = pClientCap->VendorNameLen;
	memcpy(pNewClient->ClientCapab.VendorName, 
		pClientCap->VendorName, pClientCap->VendorNameLen);
	
	pNewClient->ClientCapab.ProductTypeLen = pClientCap->ProductTypeLen;
	memcpy(pNewClient->ClientCapab.ProductType, 
		pClientCap->ProductType, pClientCap->ProductTypeLen);
	
	pNewClient->ClientCapab.ProductNameLen = pClientCap->ProductNameLen;
	memcpy(pNewClient->ClientCapab.ProductName, 
		pClientCap->ProductName, pClientCap->ProductNameLen);
#endif
	return iRet;
}

/*Init Client connect socket and query client capability information*/
int ClientInit(ClientInfo *pNewClient, UINT_8 *pIPAddr)
{
	int iRet = 0;
	LOGD("Client Init:  IP = [%s]\n", pIPAddr);
	memcpy(pNewClient->ClientIPAddr, pIPAddr, strlen(pIPAddr));

	/*socekt set*/
	pNewClient->ClientID = g_CtrlAdpter.ClientID++;
	pNewClient->SequenceNum = random();
	pNewClient->SessionID = g_CtrlAdpter.SessionID;
	pNewClient->Addr.sin_family 	 = AF_INET;
	pNewClient->Addr.sin_addr.s_addr = inet_addr(pNewClient->ClientIPAddr);
	pNewClient->Addr.sin_port		 = htons(IoT_CONTROL_PORT);

	if (SOCKTYPE_TCP == g_CtrlAdpter.SocketType)
	{
		Log_Printf("Socket type is tcp {%s]", pNewClient->ClientIPAddr);
		pNewClient->Socket = socket(AF_INET, SOCK_STREAM, 0);
		if (-1 == connect(pNewClient->Socket, 
						  (struct sockaddr_in *)&(pNewClient->Addr),
						  sizeof(struct sockaddr_in)))
		{
			Log_Printf("socket error [%s]", strerror(errno));
			return -1;
		}
	}
	else if(SOCKTYPE_UDP == g_CtrlAdpter.SocketType)
	{
		pNewClient->Socket = socket(AF_INET, SOCK_DGRAM, 0);
	}	
	
	return iRet;
}

int ClientListAdded(ClientInfo *pNewClient)
{
	int iRet = 0;
	ClientInfo *pClient;
	
	Log_Printf("New Client added Start.");
	if (NULL == g_CtrlAdpter.ClientInfoList)
	{
		g_CtrlAdpter.ClientInfoList = pNewClient;
		return 0;
	}
	
	pClient = g_CtrlAdpter.ClientInfoList;
	while (NULL != pClient->Next)
	{
		pClient = pClient->Next;
	}
	
	pClient->Next = pNewClient;
	Log_Printf("New Client added End.");
	return 0;
}

/*If input data include an IP address and other data,this function can take out the IP address only*/
UINT_8 *IpAddrTransfer(UINT_8 * pIPAddr)
{

	LOGD("IpAddrTransfer Function");
	int len;
	int i=0,j=0,k=0,m=0,count=0;
	LOGD("Input pIPAddr:= [%s]\n", pIPAddr);
	char addr[100]={0};
	char temp[100]={0};
	memcpy(addr,pIPAddr, strlen(pIPAddr));
	len=strlen(addr);
	for(m;m<len;m++)
	{
		if(addr[m]=='.')
		count++;
	}
	//LOGD("The totol number of dot is:[%d]\n", count);
	if(count<3)
	{
		LOGD("Error:Receive data do not meet the requirements!");

	}
	if(count>3)
	{
		while(i<=4)
		{
			if(addr[j]!='.')
			{
				temp[j] =addr[j];

			}

			if(addr[j]=='.')
			{
				temp[j] =addr[j];
				i++;
			}

			j++;

			if(i==4)
			{
				break;
			}
		}

		len=strlen(temp);
		temp[j-1]='\0';
		//LOGD("temp IP =[%s]\n",temp);
		pIPAddr=temp;
		LOGD("Transfered pIPAddr =[%s]\n",pIPAddr);
	}

	return   pIPAddr;
}

int NewClientAdded(void *DiscoveryInfo)
{
	UINT_8 *pNewIPAddr;
	ClientInfo *pNewClient = NULL;
	//pNewIPAddr = (UINT_8 *)DiscoveryInfo;
	
	pNewIPAddr = IpAddrTransfer((UINT_8 *)DiscoveryInfo);

	Log_Printf("Added New Client Start.\n");
	pNewClient = ClientMatched(pNewIPAddr);	
	if (NULL != pNewClient)
	{
		Log_Printf("Client has in ClientList : [%s]\n Query client capabilty",pNewIPAddr );
		if (GetClientCapability(pNewClient) < 0)
		{
			Log_Printf("GetClientCapability error ");
			return -1;
		}

		return 0;
	}
	
	pNewClient = AllocClient();
	if (NULL == pNewClient)
	{
		Log_Printf("Alloc Client Info error");
		return -1;
	}
	
	if (ClientInit(pNewClient, pNewIPAddr) < 0)
	{
		Log_Printf("Client Init error!");
		return -1;
	}
	ClientListAdded(pNewClient);

	/*query client capability*/
	if (GetClientCapability(pNewClient) < 0)
	{
		Log_Printf("GetClientCapability error ");
		return -1;
	}
	
	Log_Printf("New Client added success: [%s], sock:%d\n", 
		pNewClient->ClientIPAddr, pNewClient->Socket);
	return 0;
}

int ReceiveDataValid(void *DiscoveryInfo, int iLen)
{
	int iRet = TRUE;

	//1.1.1.1 
	if (iLen < 7)
	{
		Log_Printf("Invalid ipaddr.");
		return FALSE;
	}

	return iRet;
}


/*Device discovery thread, all clients will response ip address */
void *DeviceDiscoveryThread(void *arg)
{
	int iRet = 0;
	int iLen = 0;
	
	while (g_CtrlAdpter.DeviceDiscoveryFlag == TRUE)
	{
		memset(g_CtrlAdpter.RecvBuffer, 0, PACKAGE_BUFFER_MAX_LEN);
		iRet = ReceiveDiscoveryMessage(g_CtrlAdpter.DeviceDisRecvSocket, 
							g_CtrlAdpter.RecvBuffer, &iLen);

		if (iRet > 0 && TRUE == ReceiveDataValid(g_CtrlAdpter.RecvBuffer, iLen))
		{
			NewClientAdded(g_CtrlAdpter.RecvBuffer);
		}
		
	}
	
	pthread_exit((void *)1);
	
	return NULL;
}

UINT_32 GetClientNum(ClientInfo *pClientList)
{
	UINT_32 Number = 0;
	
	ClientInfo *pClient = NULL;
	
	pClient = pClientList;
	
	while (NULL != pClient)
	{
		Number ++;
		pClient = pClient->Next;
	}
	
	return Number;
}

ClientInfo *QueryClientInfo(UINT_32 *Count, UINT_32 NetWorkType)
{
	ClientInfo *pClient = NULL;
	pthread_t ntid;
	UINT_32 error;
	struct timeval Timeout;
	int iRet = 0;
	int iLen = 0;
	Timeout.tv_sec = DEVICE_DISCOVERY_TIMEOUT_SEC;
	Timeout.tv_usec = 0;
	char QueryData[] = "ip";
	int iTimeSock = 0;
	
	iTimeSock = socket(AF_INET, SOCK_STREAM, 0);
	pClient = g_CtrlAdpter.ClientInfoList;
//	if (InitDeviceDiscoveryServer() < 0)
//	{
//		Log_Printf("InitDeviceDiscoveryServer error.");
//		return pClient;
//	}

	/*set device discovery flag*/
	g_CtrlAdpter.DeviceDiscoveryFlag = TRUE;
	
	/*control with internet*/
	if (NETWORK_WLAN == NetWorkType)
	{
#if 0
		if (SendLogOnMessage() < 0)
		{
			Log_Printf("logon error.");
			*Count = GetClientNum(pClient);
			return pClient;
		}

		char Client[MAC_ADDR_LEN] = {0x00, 0x0c,0x43,0x12,0x34,0xdd};
		SendAddFriendMessage(Client);

		iRet = select(iTimeSock + 1, 
				NULL, NULL, NULL, &Timeout);
#endif		
	}
	else  //control in LAN
	{
		
		
		memset(g_CtrlAdpter.SendBuffer, 0, PACKAGE_BUFFER_MAX_LEN);
		iLen = strlen(QueryData);

		memcpy(g_CtrlAdpter.SendBuffer, QueryData, iLen);

		DeviceDisMessageSend(g_CtrlAdpter.DeviceDisRecvSocket,
			g_CtrlAdpter.SendBuffer, iLen);
		
		error = pthread_create(&ntid, NULL, DeviceDiscoveryThread, NULL);
		if (0 != error)
		{
			Log_Printf("pthread_create error.");
		}

		iRet = select(g_CtrlAdpter.DeviceDisRecvSocket + 1, 
				NULL, NULL, NULL, &Timeout);
	}
	
	switch(iRet)
	{
		case -1://error
			Log_Printf("Select error.");
			break;
		case 0://timeout
			Log_Printf("the time of query client info timeout\n");
			g_CtrlAdpter.DeviceDiscoveryFlag = FALSE;
			
			break;
		default:
			break;
	}

	pClient = g_CtrlAdpter.ClientInfoList;
	*Count = GetClientNum(pClient);
	Log_Printf("Count = %d", *Count);
	
	return pClient;
}

int ReceiveDiscoveryMessage(int iRecvSock, BYTE *Buf, int *iLen)
{
	struct timeval timeout;
	int iRet = 0;
	fd_set readfd;
	timeout.tv_sec = REQ_RESP_TIMEOUT_SEC;
	timeout.tv_usec = REQ_RESP_TIMEOUT_USEC;
	struct sockaddr_in Clientaddr;
	int iFromLen;
	int recvFlag = 1;
	
	do{
		timeout.tv_sec = REQ_RESP_TIMEOUT_SEC;
		timeout.tv_usec = REQ_RESP_TIMEOUT_USEC;
		FD_ZERO(&readfd);
		FD_SET(iRecvSock, &readfd);
		iRet = select(iRecvSock + 1, &readfd, NULL, NULL, &timeout);
		switch(iRet)
		{
			case -1://error
				Log_Printf("Select error.");
				break;
			case 0://timeout
				Log_Printf("Recv timeout\n");
				return -1;
				break;
			default:
				if(FD_ISSET(iRecvSock, &readfd))
				{
					iFromLen = sizeof(Clientaddr);
					iRet = recvfrom(iRecvSock, Buf, PACKAGE_BUFFER_MAX_LEN,
								0, (struct sockaddr*)&Clientaddr, &iFromLen);
					Log_Printf("recvfrom [%d] data :[%s]\n", iRet, Buf);
					{
						int i = 0;
						for (i = 0; i < iRet; i ++)
						{
							Log_Printf("0x%02x ", Buf[i]);
						}
						Log_Printf("\n");
					}
					if (iRet > 0)
					{		
						*iLen = iRet;
						return iRet;						
					}
				}
				break;
		}
	}while(recvFlag);
	return iRet;
}


int ReceiveMessage(ClientInfo *pClient,
						BYTE *Buf,
						UINT_32 *iLen,
						BYTE Type,
						BYTE SubType
						)
{
	struct timeval timeout;
	struct timeval old_time;
	struct timeval now_time;
	
	int iRet = 0;
	IoTCtrlProtocolHeader *pMessageHeader = NULL;
	fd_set readfd;
	timeout.tv_sec = REQ_RESP_TIMEOUT_SEC;
	timeout.tv_usec = REQ_RESP_TIMEOUT_USEC;
	struct sockaddr_in Clientaddr;
	int iFromLen;
	int recvFlag = 1;

	Log_Printf("ReceiveMessage [fd = %d] \n", pClient->Socket);
	do{
		iRet = gettimeofday(&old_time, NULL);
		timeout.tv_sec = REQ_RESP_TIMEOUT_SEC;
		timeout.tv_usec = REQ_RESP_TIMEOUT_USEC;
		FD_ZERO(&readfd);
		FD_SET(pClient->Socket, &readfd);
		iRet = select(pClient->Socket + 1, &readfd, NULL, NULL, &timeout);
		switch(iRet)
		{
			case -1://error
				Log_Printf("Select error.");
				break;
			case 0://timeout
				Log_Printf("Recv timeout\n");
				return -1;
				break;
			default:
				if(FD_ISSET(pClient->Socket, &readfd))
				{
					iFromLen = sizeof(Clientaddr);
					if (SOCKTYPE_TCP == g_CtrlAdpter.SocketType)
					{
						iRet = read(pClient->Socket, Buf, PACKAGE_BUFFER_MAX_LEN);
					}
					else if (SOCKTYPE_UDP == g_CtrlAdpter.SocketType)
					{
						iRet = recvfrom(pClient->Socket, Buf, PACKAGE_BUFFER_MAX_LEN, 
							0, (struct sockaddr*)&Clientaddr, &iFromLen);
					}
					if (iRet > 0)
					{
						*iLen = iRet;
						Log_Printf("recv %d data\n", iRet);
						
//#ifdef CTRL_DATA_ENCRYPT
						/*
						 *	Control protocol:
						 *	| Magic  | RA | SA |SessionID |Sequence |Flag |Type|SubType |Data |
						 *	|<- Not encrypt ->|<-						Encrypt 							->|
						 *	Note : not encrypt RA and SA for internet control (tmp protocol)
						 */

						char cPlain[PACKAGE_BUFFER_MAX_LEN] = {0};
						BYTE cKey[16] =  {'M', 'c', 'd', 'w', 'C', 'n', 'w', 'C', 'd', 's', 's', '2', '_', '1', '8', 'p'};
						int iBlockCount = 0;
						int iChiperLen = AES_BLOCK_SIZES;
						int i = 0;
						char *pEncryptStart = NULL;
						int iNotEncryptLen = 0;
						
						if (1 == g_CtrlAdpter.DataEncrpty)
						{
							iNotEncryptLen = 4 + 6 + 6; //Magic len + Revice MAC Len + Send MAC Len
							if (iRet < iNotEncryptLen)
							{
								Log_Printf("invaild data.");
								break;
							}
							pEncryptStart = Buf + iNotEncryptLen;
							iBlockCount = (iRet - iNotEncryptLen + AES_BLOCK_SIZES - 1)/AES_BLOCK_SIZES;
							for (i = 0;i < iBlockCount; i ++)
							{
								RT_AES_Decrypt(pEncryptStart + i*AES_BLOCK_SIZES, AES_BLOCK_SIZES, 
									g_CtrlAdpter.SecurityKey, AES_BLOCK_SIZES, cPlain + i*AES_BLOCK_SIZES, &iChiperLen);
							}
							
							memcpy(pEncryptStart, cPlain, iBlockCount * AES_BLOCK_SIZES);		
							*iLen = iNotEncryptLen + iBlockCount * AES_BLOCK_SIZES;
						}
//#endif					
						MessageDump(Buf, iRet);
						pMessageHeader = (IoTCtrlProtocolHeader *)Buf;
						if ((Type != pMessageHeader->Type) || 
							(SubType != pMessageHeader->SubType))
						{
							Log_Printf("Drop the message : Type %d not matched %d or subtype %d not matched %d", 
								pMessageHeader->Type, Type, pMessageHeader->SubType, SubType);
/*
							iRet = gettimeofday(&now_time, NULL);
							if (now_time.tv_usec < old_time.tv_usec)
							{
								timeout.tv_sec = now_time.tv_sec - old_time.tv_sec - 1;
								timeout.tv_usec = now_time.tv_usec - old_time.tv_usec + 1000000;
							}
							else
							{
								timeout.tv_sec = now_time.tv_sec - old_time.tv_sec;
								timeout.tv_usec = now_time.tv_usec - old_time.tv_usec;
							}
*/
							break;
						}
						
						//We must wait the expected message.And we need a timeout in there. 
						if (pClient->SequenceNum != pMessageHeader->Sequence)
						{
							Log_Printf("Drop the message: Sequence Number is Not matched\n");
							break;
						}						
						
						return iRet;
						
					}
				}
				break;
		}
	}while(recvFlag);

	return iRet;

}

int DeviceDisMessageSend(int iSendSocket, BYTE *Data, int iLen)
{
	int iRet = 0;
	int i = 0, j = 0;
	int iTimes = 2;
	int iCount = 2;
	
	//we send the message one times, if we does not recv response, controller will retransmit this message.
	//we don't care it
	for (i = 0; i < iTimes; i++)
	{
		for (j = 0; j < iCount; j ++)
		{
			//iRet = write(iSendSocket, Data, iLen);
			iRet = sendto(iSendSocket, Data, iLen, 0, 
						(struct sockaddr *)&g_CtrlAdpter.DeviceDisSendAddr, 
						sizeof(struct sockaddr_in));
			//Log_Printf("write error.[%s]\n", strerror(errno));
			usleep(200000); //sleep 1ms
		}
		usleep(200000);
	}
	if (iRet < 0)
	{
		Log_Printf("write error.\n");
	}
	
	return iRet;
	
}


int SendMessage(ClientInfo *pClient,
					 BYTE *Data,
					 int iLen)
{
	int iRet = 0;
	int i = 0, j = 0;
	int iTimes = 1;
	int iCount = 1;
	Log_Printf("write [iLen = %d] data.\n", iLen);
	//we send the message one times, if we does not recv response, controller will retransmit this message.
	//we don't care it

//#ifdef CTRL_DATA_ENCRYPT
	/*
	 *  Control protocol:
	 *  | Magic  | RA | SA |SessionID |Sequence |Flag |Type|SubType |Data |
	 *  |<- Not encrypt ->|<-                       Encrypt                             ->|
	 *  Note : not encrypt RA and SA for internet control (tmp protocol)
	 */
	char cCipher[PACKAGE_BUFFER_MAX_LEN] = {0};
	BYTE cKey[16] =  {'M', 'c', 'd', 'w', 'C', 'n', 'w', 'C', 'd', 's', 's', '2', '_', '1', '8', 'p'};
	int iBlockCount = 0;
	int iChiperLen = AES_BLOCK_SIZES;
	char *pEncryptStart = NULL;
	int iNotEncryptLen = 0;

	if (1 == g_CtrlAdpter.DataEncrpty)
	{
		iNotEncryptLen = 4 + 6 + 6; //Magic len + Revice MAC Len + Send MAC Len
		pEncryptStart = Data + iNotEncryptLen;
		iBlockCount = (iLen - iNotEncryptLen + AES_BLOCK_SIZES - 1)/AES_BLOCK_SIZES;
		for (i = 0;i < iBlockCount; i ++)
		{
			RT_AES_Encrypt(pEncryptStart + i*AES_BLOCK_SIZES, AES_BLOCK_SIZES, 
				g_CtrlAdpter.SecurityKey, AES_BLOCK_SIZES, cCipher + i*AES_BLOCK_SIZES, &iChiperLen);
		}

		memcpy(pEncryptStart, cCipher, iBlockCount * AES_BLOCK_SIZES);
		iLen = iNotEncryptLen + iBlockCount * AES_BLOCK_SIZES;
	}
//#endif
	for (i = 0; i < iTimes; i++)
	{
		for (j = 0; j < iCount; j ++)
		{
			if (SOCKTYPE_TCP == g_CtrlAdpter.SocketType)
			{
				iRet = write(pClient->Socket, Data, iLen);
				if (iRet < 0)
				{
					Log_Printf("write error.\n");
				}
				Log_Printf("write [%d] data.\n", iRet);
			}
			else if (SOCKTYPE_UDP == g_CtrlAdpter.SocketType)
			{
				iRet = sendto(pClient->Socket, Data, iLen, 0, 
						(struct sockaddr *)&pClient->Addr, sizeof(struct sockaddr_in));
				if (iRet < 0)
				{
					Log_Printf("sendto error.\n");
				}
			}
			usleep(1000); //sleep 1ms
		}
		usleep(20000);
	}
	
	
	return iRet;
	
}

int SendAddFriendMessage(char *PFriendID)
{
	int iRet = 0;
	char AddfriendPrefix[] = "addfriend:";
	char DataBuffer[64] = {0};
	int iLen = 0;
	int FriendIDLen = MAC_ADDR_LEN;
	
	memcpy(DataBuffer, AddfriendPrefix, sizeof(AddfriendPrefix)-1);
	iLen += sizeof(AddfriendPrefix)-1;
	memcpy(DataBuffer + iLen, PFriendID, FriendIDLen);
	iLen += FriendIDLen;
	
#if 0
	char cCipher[64] = {0};
	BYTE cKey[16] =  {'M', 'c', 'd', 'w', 'C', 'n', 'w', 'C', 'd', 's', 's', '2', '_', '1', '8', 'p'};
	int iBlockCount = 0;
	int iChiperLen = AES_BLOCK_SIZES;
	
	iBlockCount = (iLen + AES_BLOCK_SIZES - 1)/AES_BLOCK_SIZES;
	for (i = 0;i < iBlockCount; i ++)
	{
		RT_AES_Encrypt(DataBuffer + i*AES_BLOCK_SIZES, AES_BLOCK_SIZES, 
			cKey, AES_BLOCK_SIZES, cCipher + i*AES_BLOCK_SIZES, &iChiperLen);
	}

	DataBuffer = cCipher;
	iLen = iBlockCount * AES_BLOCK_SIZES;
#endif

	iRet = write(g_CtrlAdpter.ControlServSocket, DataBuffer, iLen);
	if (iRet < 0)
	{
		Log_Printf("write error.\n");
		return iRet;
	}
	int i = 0;
	for (i = 0; i < MAC_ADDR_LEN; i ++)
	{
		Log_Printf("0x%x", PFriendID[i]);
	}
	Log_Printf("\nwrite [%d] data.\n", iRet);
	return iRet;
}

static int StrToHex(char *pStr, unsigned int *Result)
{
	char *pWork = NULL;
	unsigned int uiLen = 0;
	unsigned int i = 0;
	unsigned int uiTmp = 1;
	unsigned int uiNum = 0;
	unsigned int uiRst = 0;
	
	if (NULL == pStr)
	{
		return -1;
	}

	uiLen = strlen(pStr);
	pWork = pStr + uiLen - 1;
	
	for (i = 0; i < uiLen; i ++)
	{
		if ((*pWork >= '0' && *pWork <= '9'))
		{
			uiNum = *pWork- '0';
		}
		else if (*pWork >= 'A' && *pWork <= 'F')
		{
			uiNum = *pWork- 'A' + 10;
		}
		else if (*pWork >= 'a' && *pWork <= 'f')
		{
			uiNum = *pWork- 'a' + 10;
		}
		else
		{
			return -1;
		}
			
		uiRst += uiNum * uiTmp;
		uiTmp *= 16;
		pWork --;
	}

	*Result = uiRst;

	return 0;
}

/*converter "11:22:33:4a:5b:6c" to Byte list 0x11,0x22,0x33,0x4a,0x5b,0x6c*/
static int MACStrToByte(char *pMacStr, char *pMacByte)
{
	char *pWork = NULL;
	char *Tmp = NULL;
	char Buf[16] = {0};
	int ByteCount = 0;
	
	pWork = pMacStr;
	while ('\0' != *pWork)
	{
		Tmp = Buf;
		memset(Tmp, 0, sizeof(Buf));
		while ((':' != *pWork) && ('\0' != *pWork))
		{
			*(Tmp ++) = *pWork;
			pWork ++;
		}

		//input invalid
		if (2 != strlen(Buf))
		{
			return -1;
		}
		
		if (-1 == StrToHex(Buf, &pMacByte[ByteCount]))
		{
			return -1;
		}
		ByteCount ++;
		if ('\0' != *pWork)
		{
			pWork ++;
		}
	}

	if (MAC_ADDR_LEN != ByteCount)
	{
		return -1;
	}

	return 0;
}


int AddFriend(const char *pFriendID)
{
#if 0
	int iRet = 0;
	char *pWork = NULL;
	char Buf[16] = {0};
	char ClientID[MAC_ADDR_LEN] = {0x00, 0x0c,0x43,0x12,0x34,0xdd};
	char *Tmp = NULL;
	int i = 0;
	
	if (NULL == pFriendID)
	{
		Log_Printf("pFriendID is NULL.\n");
		return -1;
	}

	pWork = pFriendID;
	while ('\0' != *pWork)
	{
		Tmp = Buf;
		memset(Tmp, 0, sizeof(Buf));
		while ((':' != *pWork) && ('\0' != *pWork))
		{
			*(Tmp ++) = *pWork;
			pWork ++;
		}
		ClientID[i++] = StrToHex(Buf);
		if ('\0' != *pWork)
		{
			pWork ++;
		}
	}
	
	Log_Printf("FriendID is [%02x:%02x:%02x:%02x:%02x:%02x]\n", ClientID[0],
		ClientID[1],ClientID[2],ClientID[3],ClientID[4],ClientID[5]);
	iRet = SendAddFriendMessage(ClientID);
#endif
	char ClientID[MAC_ADDR_LEN] = {0x00, 0x0c,0x43,0x12,0x34,0xdd};
	char Default[MAC_ADDR_LEN] = {0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF};

	int iRet = 0;

	if (-1 == MACStrToByte(pFriendID, ClientID))
	{
		Log_Printf("Friend ID is invalid. we use Default FF:FF:FF:FF:FF:FF replace it");
		memcpy(ClientID, Default, MAC_ADDR_LEN);
	}
	
	Log_Printf("FriendID is [%02x:%02x:%02x:%02x:%02x:%02x]\n", ClientID[0],
			ClientID[1],ClientID[2],ClientID[3],ClientID[4],ClientID[5]);
	iRet = SendAddFriendMessage(ClientID);

	return iRet;
}

int FriendInit(ClientInfo *pNewClient, char *FriendID)
{
	int iRet = 0;
	
	Log_Printf("Friend Init:");
	
	/*socekt set*/
	pNewClient->ClientID = g_CtrlAdpter.ClientID++;
	pNewClient->SequenceNum = random();
	pNewClient->SessionID = g_CtrlAdpter.SessionID;	

	/*all message will send to control server on internel*/
	pNewClient->Socket = g_CtrlAdpter.ControlServSocket;
	memcpy(pNewClient->ClientMACAddr, FriendID, MAC_ADDR_LEN);
	
	int i = 0;
	for (i = 0; i < MAC_ADDR_LEN; i ++)
	Log_Printf("[%#x]", FriendID[i] & 0xFF);
	/*query client capability*/
	if (GetClientCapability(pNewClient) < 0)
	{
		Log_Printf("GetClientCapability error ");
		return -1;
	}
	
	return iRet;
}

ClientInfo *FindClient(char *FriendID)
{
//	int iRet = FALSE;
	ClientInfo *pClient = NULL;
	pClient = g_CtrlAdpter.ClientInfoList;
	
	while (NULL != pClient)
	{
		if (!memcmp(pClient->ClientMACAddr, FriendID, MAC_ADDR_LEN))
		{
			return pClient;
		}
		
		pClient = pClient->Next;
	}
	
	return NULL;
}

int FriendOnline(char *FriendID)
{
	int FriendIDLen = MAC_ADDR_LEN;
	ClientInfo *pClient = NULL;
	
	if (NULL == FriendID)
	{
		Log_Printf("FriendID Error");
		return -1;
	}

	if (NULL != FindClient(FriendID))
	{
		Log_Printf("Friend has online [%s]", FriendID);
		return -1;
	}

	pClient = AllocClient();
	if (NULL == pClient)
	{
		Log_Printf("Alloc Client Info error");
		return -1;
	}

	if (FriendInit(pClient, FriendID) < 0)
	{
		Log_Printf("Friend Init error");
		return -1;
	}
	
	ClientListAdded(pClient);
	Log_Printf("New Client added success.");
	return 0;
}

int FriendOffLine(char *FriendID)
{
	ClientInfo *pClient = NULL;
		
	if (NULL == FriendID|| strlen(FriendID) < FriendID)
	{
		Log_Printf("FriendID Error");
		return -1;
	}

	if (NULL == (pClient = FindClient(FriendID)))
	{
		Log_Printf("Friend has not online, does not offline [%s]", FriendID);
		return -1;
	}

	if (DeallocClient(pClient) < 0)
	{
		Log_Printf("DeallocClient error");
		return -1;
	}

	return 0;
}

int MessageDump(char *Buf, int iLen)
{
	int i = 0;
	
	for (i = 0; i < iLen; i ++)
	{
		Log_Printf("[%#x]", Buf[i]);
	}

	return 0;
}

int ServMessageProcess(char *Buffer, int iLen)
{
	char LogonPrefix[] = "userlogon:";
	char LogoffPrefix[] = "userlogoff:";
	char logonFlag = 0;
	char logoffFlag = 0;
	char *FriendId = NULL;

	if (NULL == Buffer)
	{
		Log_Printf("Buffer = NULL\n");
		return -1;
	}
	logonFlag = memcmp(Buffer, LogonPrefix, sizeof(LogonPrefix)-1);
	logoffFlag = memcmp(Buffer, LogoffPrefix, sizeof(LogoffPrefix)-1);

	if (0 != logonFlag && 0 != logonFlag)
	{
		Log_Printf("Recv not logon or logoff message");
		return -1;
	}

	if (0 == logonFlag)
	{
		FriendId = Buffer + sizeof(LogonPrefix) - 1;
		FriendOnline(FriendId);
	}

	if (0 == logoffFlag)
	{
		FriendId = Buffer + sizeof(LogoffPrefix) - 1;
		FriendOffLine(FriendId);
	}
	
	return 0;
}

void *RecvCtrlServInfo(void *argv)
{
	struct timeval timeout;
	int iRet = 0;
	fd_set readfd;
	timeout.tv_sec = REQ_RESP_TIMEOUT_SEC;
	timeout.tv_usec = REQ_RESP_TIMEOUT_USEC;
	struct sockaddr_in Clientaddr;
	int iFromLen;
	int recvFlag = 1;
	int iRecvSock = g_CtrlAdpter.ControlServSocket;
	char *Buf = (char *)g_CtrlAdpter.RecvBuffer;

	Log_Printf("thread enter.[fd = %d]", g_CtrlAdpter.ControlServSocket);

	do{
		timeout.tv_sec = REQ_RESP_TIMEOUT_SEC;
		timeout.tv_usec = REQ_RESP_TIMEOUT_USEC;
		FD_ZERO(&readfd);
		FD_SET(iRecvSock, &readfd);
		iRet = select(iRecvSock + 1, &readfd, NULL, NULL, &timeout);
		switch(iRet)
		{
			case -1://error
				Log_Printf("Select error.");
				break;
			case 0://timeout
				Log_Printf("RecvCtrlServInfo Recv timeout\n");
//				pthread_exit((void *)1);
//				return -1;
				break;
			default:
				if(FD_ISSET(iRecvSock, &readfd))
				{
					iFromLen = sizeof(Clientaddr);
					if (SOCKTYPE_TCP == g_CtrlAdpter.SocketType)
					{
						iRet = read(iRecvSock, Buf, PACKAGE_BUFFER_MAX_LEN);
					}
					else if (SOCKTYPE_UDP == g_CtrlAdpter.SocketType)
					{
						iRet = recvfrom(iRecvSock, Buf, PACKAGE_BUFFER_MAX_LEN, 
							0, (struct sockaddr*)&Clientaddr, &iFromLen);
					}
					if (iRet > 0)
					{			
						Log_Printf("Recv Serv message [%d] : [%s]",iRet , Buf);
						MessageDump(Buf, iRet);	
						ServMessageProcess(Buf, iRet);
					//	return iRet;		
					}
				}
				break;
		}
	}while(TRUE);

	//Need kill this thread when main thread exit
	Log_Printf("thread exit.[fd = %d]", g_CtrlAdpter.ControlServSocket);

	pthread_exit((void *)1);

	return 0;
}

int RecvClientLogOn(void)
{
	int error = 0;
	pthread_t ntid;

	error = pthread_create(&ntid, NULL, RecvCtrlServInfo, NULL);
	if (0 != error)
	{
		Log_Printf("pthread_create error.");
		return -1;
	}

	return 0;
}

int SendLogOnMessage(void)
{
	int iRet = 0;
	char LogonPrefix[] = "userlogon:";
	char DataBuffer[32] = {0};
	int iLen = 0;

	
	memcpy(DataBuffer, LogonPrefix, sizeof(LogonPrefix)-1);
	iLen += sizeof(LogonPrefix)-1;
	memcpy(DataBuffer + iLen, g_CtrlAdpter.OwnMACAddress, MAC_ADDR_LEN);
	iLen += MAC_ADDR_LEN;
	
	RecvClientLogOn();
#if 0
		char cCipher[64] = {0};
		BYTE cKey[16] =  {'M', 'c', 'd', 'w', 'C', 'n', 'w', 'C', 'd', 's', 's', '2', '_', '1', '8', 'p'};
		int iBlockCount = 0;
		int iChiperLen = AES_BLOCK_SIZES;
		
		iBlockCount = (iLen + AES_BLOCK_SIZES - 1)/AES_BLOCK_SIZES;
		for (i = 0;i < iBlockCount; i ++)
		{
			RT_AES_Encrypt(DataBuffer + i*AES_BLOCK_SIZES, AES_BLOCK_SIZES, 
				cKey, AES_BLOCK_SIZES, cCipher + i*AES_BLOCK_SIZES, &iChiperLen);
		}
	
		DataBuffer = cCipher;
		iLen = iBlockCount * AES_BLOCK_SIZES;
#endif

	iRet = write(g_CtrlAdpter.ControlServSocket, DataBuffer, iLen);
	if (iRet < 0)
	{
		Log_Printf("write error.\n");
		return iRet;
	}
	Log_Printf("write [%d] data.\n", iRet);
//	RecvCtrlServInfo(NULL);
	
	return iRet;
}

int InitControlEnv(char *pMacAddr)
{
	UINT_8 MACAddress[MAC_ADDR_LEN] = {0x76, 0x81, 0x76, 0x81, 0x76, 0x81};
	UINT_8 DefaultMac[MAC_ADDR_LEN] = {0x76, 0x81, 0x76, 0x81, 0x76, 0x81};
	char szPlain[AES_BLOCK_SIZES] = {0};
	char szKey[AES_BLOCK_SIZES] = 	{0x10, 0x22, 0x3F, 0x03,
									 0xDA, 0x1F, 0xA3, 0x55,
									 0x31, 0x42, 0x06, 0x67,
									 0x82, 0x9A, 0x12, 0x53};
	int iChiperLen = AES_BLOCK_SIZES;
	int iRand = 0;
#if 0
	char Device[] = "wlan0";
	struct ifreq req;
	
	memset(&g_CtrlAdpter, 0, sizeof(g_CtrlAdpter));

	int Sock = socket(AF_INET, SOCK_DGRAM, 0);
	memcpy(req.ifr_name, Device, strlen(Device));
	int ErrNo = ioctl(Sock, SIOCGIFHWADDR, &req, sizeof(req));
	
	if (-1 != ErrNo)
	{
		memcpy(MACAddress, req.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
	}
	else
	{
		Log_Printf("error: %s", strerror(errno));
	}
	close(Sock);
#endif
	if (NULL != pMacAddr)
	{
		if (-1 == MACStrToByte(pMacAddr, MACAddress))
		{
			Log_Printf("MAC is invalid. we use Default MAC to replace it");
			memcpy(MACAddress, DefaultMac, MAC_ADDR_LEN);
		}
		Log_Printf("MAC ADDR is [%02x:%02x:%02x:%02x:%02x:%02x]\n", MACAddress[0],
			MACAddress[1],MACAddress[2],MACAddress[3],MACAddress[4],MACAddress[5]);
	}
	/*init own mac address*/
	memcpy(g_CtrlAdpter.OwnMACAddress, MACAddress, MAC_ADDR_LEN);

#if 0
	szPlain[5] = 0x22;
	szPlain[10] = 0x33;
	szPlain[15] = 0x44;
	szKey[3] = 0x11;
	szKey[9] = 0x12;
	szKey[15]= 0x13;

	/*init own mac address*/
	memcpy(g_CtrlAdpter.OwnMACAddress, MACAddress, MAC_ADDR_LEN);
	g_CtrlAdpter.ClientInfoList = NULL;

	/*init aes encrypt key*/
	RT_AES_Encrypt(szPlain, AES_BLOCK_SIZES, 
			szKey, AES_BLOCK_SIZES, g_CtrlAdpter.SecurityKey , &iChiperLen);
#endif
	memcpy(g_CtrlAdpter.SecurityKey, szKey, AES_BLOCK_SIZES);
	return 0;
}


int InitInternetControlServer(const char *IPAddr, const char *MACAddr)
{
	int iRet = 0;
	
	InitControlEnv(MACAddr);

	g_CtrlAdpter.ControlServSocket = socket(AF_INET, SOCK_STREAM, 0);
	g_CtrlAdpter.NetworkType = NETWORK_WLAN;
	g_CtrlAdpter.ControlServAddr.sin_family 	 = AF_INET;
	g_CtrlAdpter.ControlServAddr.sin_addr.s_addr = inet_addr(IPAddr);
	g_CtrlAdpter.ControlServAddr.sin_port		 = htons(IoT_CONTROL_PORT);

	iRet = connect(g_CtrlAdpter.ControlServSocket, 
				   (struct sockaddr_in *)&(g_CtrlAdpter.ControlServAddr),
				   sizeof(struct sockaddr_in));
	if (-1 == iRet)
	{
		Log_Printf("socket error [%s]", strerror(errno));
		return -1;
	}
	else
	{
		Log_Printf("Connected to Control server [%s:%d], fd=%d", 
			IPAddr, IoT_CONTROL_PORT, g_CtrlAdpter.ControlServSocket);
	}

	struct sockaddr_in add;

	int addr_len = sizeof(add);
	getsockname(g_CtrlAdpter.ControlServSocket, (struct sockaddr *)&add, &addr_len);
	Log_Printf("Local port [%d]", ntohs(add.sin_port));

	if (SendLogOnMessage() < 0)
	{
		Log_Printf("SendLogOnMessage error.");
//		*Count = GetClientNum(pClient);
		return iRet;
	}
	
	return iRet;
}


/*
 * Description : Init the socket of Device discovery server
 * 			 
 * Use udp broadcast send device discovery request, then receive client response by udp unicast.
 *
 */
int InitDeviceDiscoveryServer(const char *MACAddr)
{
	int iRet = 0;
	
	InitControlEnv(MACAddr);
	/*device discovery message recv socket addr*/
	g_CtrlAdpter.DeviceDisRecvSocket = socket(AF_INET, SOCK_DGRAM, 0);
	g_CtrlAdpter.NetworkType = NETWORK_LAN;

	g_CtrlAdpter.DeviceDisRecvAddr.sin_family      = AF_INET;
	g_CtrlAdpter.DeviceDisRecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	g_CtrlAdpter.DeviceDisRecvAddr.sin_port        = htons(DEVICE_DISCOVERY_PORT);
	iRet = bind(g_CtrlAdpter.DeviceDisRecvSocket, 
			(struct sockaddr*)&(g_CtrlAdpter.DeviceDisRecvAddr), 
			sizeof(g_CtrlAdpter.DeviceDisRecvAddr));
	
	if (iRet < 0)
	{
		Log_Printf("Device discovery message recv socket init error!");
		return -1;
	}

	/*device discovery message send socket addr*/
	g_CtrlAdpter.DeviceDisSendSocket = socket(AF_INET, SOCK_DGRAM, 0);
	g_CtrlAdpter.DeviceDisSendAddr.sin_family      = AF_INET;
	g_CtrlAdpter.DeviceDisSendAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	g_CtrlAdpter.DeviceDisSendAddr.sin_port        = htons(DEVICE_DISCOVERY_PORT);

	int on = 1;
	iRet |= setsockopt(g_CtrlAdpter.DeviceDisSendSocket , SOL_SOCKET, SO_BROADCAST, 
			&on, sizeof(on));

	iRet |= setsockopt(g_CtrlAdpter.DeviceDisRecvSocket, SOL_SOCKET, SO_BROADCAST, 
			&on, sizeof(on));
	
	if (iRet < 0)
	{
		Log_Printf("Device discovery message send socket setsockoopt error!\n");
		return -1;
	}

	return iRet;
}


#if 0
/*
 * Description : Init the socket of TCP server
 * @iType : 0 Broadcast
 * 			1 Muticast
 * 			 
 * 
 */
int InitControlServer(const char *IPaddr, int iPort)
{
	int iRet = 0;
	
	g_SequenceNum = random();

	if (NULL == IPaddr)
	{
		Log_Printf("IPAddr is NULL");
		return -1;
	}
	
	g_sTCPLink = socket(AF_INET, SOCK_STREAM, 0);
	memset((void*)&g_ServAddr, 0, sizeof(struct sockaddr_in));
	Log_Printf("IPAddr = %s", IPaddr);
	Log_Printf("iPort = %d", iPort);

	g_ServAddr.sin_family = AF_INET;
	g_ServAddr.sin_addr.s_addr = inet_addr(IPaddr);
	g_ServAddr.sin_port = htons(iPort);

	if (-1 == connect(g_sTCPLink, (struct sockaddr_in *)&g_ServAddr,	sizeof(struct sockaddr_in)))
	{
		Log_Printf("socket error");
		return -1;
	}

	return iRet;
}
#endif

#if 0
/*
 * Description : Init the socket of control and monitor
 * @iType : 0 Broadcast
 * 			1 Muticast
 * 			 
 * 
 */
int InitControlServer(int iType)
{
	int iRet = 0;
	
	g_SequenceNum = random();
	
	memset((void*)&g_MoniotrAddr, 0, sizeof(struct sockaddr_in));
	memset((void*)&g_ControlAddr, 0, sizeof(struct sockaddr_in));
	
	g_sControl = socket(AF_INET, SOCK_DGRAM, 0);
	g_sMonitor = socket(AF_INET, SOCK_DGRAM, 0);
	if (g_sControl < 0 || g_sMonitor < 0)
	{
		Log_Printf("socket error");
		return -1;
	}
	Log_Printf("g_sControl = %d\n", g_sControl);
	Log_Printf("g_sMonitor = %d\n", g_sMonitor);
	
	switch (iType)
	{
		case CONTROL_USE_BROADCAST:
			/*Monitor setup*/
			g_MoniotrAddr.sin_family = AF_INET;
			g_MoniotrAddr.sin_addr.s_addr = htons(INADDR_ANY);
			g_MoniotrAddr.sin_port = htons(IoT_CONTROL_PORT);
			iRet = bind(g_sMonitor, (struct sockaddr*)&g_MoniotrAddr, sizeof(g_MoniotrAddr));
			if (iRet < 0)
			{
				Log_Printf("Mointor socket bind error");
				return -1;
			}
	
			/*Control setup*/
			g_ControlAddr.sin_family = AF_INET;
			g_ControlAddr.sin_port = htons(IoT_CONTROL_PORT);
			g_ControlAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

			int on = 1;
			iRet = setsockopt(g_sControl, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
			if (iRet < 0)
			{
				Log_Printf("setsockopt error!\n");
				return -1;
			}
			break;
		case CONTROL_USE_MUTICAST:
			/*Monitor setup*/
			g_MoniotrAddr.sin_family = AF_INET;
			g_MoniotrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			g_MoniotrAddr.sin_port = htons(IoT_CONTROL_PORT);

			iRet = bind(g_sMonitor, (struct sockaddr*)&g_MoniotrAddr, sizeof(g_MoniotrAddr));
			if(iRet < 0)
			{
				Log_Printf("bind() error");
				return -2;
			}
	
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = inet_addr(MUTICAST_IP_ADDR);
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			iRet = setsockopt(g_sMonitor, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
			if (iRet < 0)
			{
				Log_Printf("setsockopt():IP_ADD_MEMBERSHIP");
				return -4;
			}
		
			/*Control setup*/
			g_ControlAddr.sin_family = AF_INET;
			g_ControlAddr.sin_addr.s_addr = inet_addr(MUTICAST_IP_ADDR);
			g_ControlAddr.sin_port = htons(IoT_CONTROL_PORT);
		
			break;
		default:
			break;
	}
	
	return 0;
}
#endif

/*
int main(int argc, char *argv[])
{
	int opt = 0;
	int iRet = 0;

	int iClientFlag = 0;
	int iServerFlag = 0;
	int iLetOn = 0;
	int iLetOff = 0;
	
	char BroadMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//	char SendMAC[MAC_ADDR_LEN] = {};
	unsigned int Seq = 0x00FF;
	CommandType cmdType = MANAGEMENT;
	ManagementCommand mgmtType = QUERY_CAPAB_REQUEST;
	unsigned int iLen = 0;
	
	if (argc < 3)
	{
		Log_Printf("Usage: %s -s|-c|-o|-f [MAC Address]\n", argv[0]);
		return -1;
	}
	
	memcpy(g_OwnMACAddress, argv[2], MAC_ADDR_LEN);
	memset(&g_ClientCapab, 0, sizeof(g_ClientCapab));

	g_ClientCapab.VendorNameLen = strlen("Mediatek");
	memcpy(g_ClientCapab.VendorName, "Mediatek", g_ClientCapab.VendorNameLen);
	
	g_ClientCapab.ProductTypeLen = strlen("IoT device");
	memcpy(g_ClientCapab.ProductType, "IoT device", g_ClientCapab.ProductTypeLen);
	
	g_ClientCapab.ProductNameLen = strlen("MT7681");
	memcpy(g_ClientCapab.ProductName, "MT7681", g_ClientCapab.ProductNameLen);
	
	iRet = InitControlServer(CONTROL_USE_BROADCAST);
	if (iRet < 0)
	{
		Log_Printf("InitControlServer() error.");
		return iRet;
	}
//		UINT_8 PackageBuild(void *Buffer, BYTE *ReceiveMAC, BYTE *SendMAC, UINT_16 SequenceNum,
//					CommandType Type, UINT_8 SubType, BYTE *Data, UINT_32 DataLen)
	
	while ((opt = getopt(argc, argv, "csof")) != -1) {
        
        switch(opt) {
			case 'c':
				iClientFlag = 1;
				break;
			case 's':
				iServerFlag = 1;
				break;
			case 'o':
				iLetOn = 1;
				break;
			case 'f':
				iLetOff = 1;
				break;
				
			default :
				break;
		}
	}
	
	if (1 == iLetOff)
	{
		UINT_16 PWMInfo[3] = {0};
		UINT_32 PackageLen = 0;
		
		PWMInfo[0] = 0;
		PWMInfo[1] = 0;
		PWMInfo[2] = 0;
	
	PackageLen = SetPWMRequestBuild(g_SendBuffer, g_OwnMACAddress, g_OwnMACAddress, 
						g_SequenceNum, (UINT_8 *)PWMInfo, 6);
		while (1)
		{
			SendMessage(g_SendBuffer, PackageLen);
			usleep(200000);
		}
	}
	
	if (1 == iLetOn)
	{
		UINT_16 PWMInfo[3] = {0};
		UINT_32 PackageLen = 0;
		
		PWMInfo[0] = 5;
		PWMInfo[1] = 5;
		PWMInfo[2] = 5;
	
		PackageLen = SetPWMRequestBuild(g_SendBuffer, g_OwnMACAddress, g_OwnMACAddress, 
							g_SequenceNum, (UINT_8 *)PWMInfo, 6);
		while (1)
		{	
			SendMessage(g_SendBuffer, PackageLen);
			usleep(200000);
		}
	}
	
	if (1 == iClientFlag)
	{
		g_QueryFlag = 1;
		memset(g_RecvBuffer, 0, PACKAGE_BUFFER_MAX_LEN);
		iRet = ReceiveMessage(g_RecvBuffer, &iLen, MANAGEMENT, QUERY_CAPAB_RESPONSE);
		int i = 0;
//		for (i = 0; i < iLen; i ++)
//		{
//			Log_Printf("%#x\n", g_RecvBuffer[i]);
//		}
//
		MessageSanity(g_RecvBuffer, iLen);
	}
	
	if (1 == iServerFlag)
	{
		
		ClientInfo *pList = NULL;
		
		pList = QueryClientInfo(&iLen);
		
		while (NULL != pList)
		{
			Log_Printf("=============== ClientID = %d\n", pList->ClientID);
			pList = pList->Next;
		}
		
//		int i = 0;
		
//		memset(g_SendBuffer, 0, PACKAGE_BUFFER_MAX_LEN);
//		iLen = QueryClientCapabilityRequestBuild((void *)g_SendBuffer, BroadMAC, g_OwnMACAddress, g_SequenceNum);
//		for (i = 0 ; i < 5; i ++)
//		{
			SendMessage(g_SendBuffer, iLen);
//		}
		
		while (1)
		{
			memset(g_RecvBuffer, 0, PACKAGE_BUFFER_MAX_LEN);
			iRet = ReceiveMessage(g_RecvBuffer, &iLen);

			for (i = 0; i < iLen; i ++)
			{
				Log_Printf("%#x\n", g_RecvBuffer[i]);
			}
			
			iRet = MessageSanity(g_RecvBuffer, iLen);
			if (0 == iRet)
			{
				for (i = 0; i < iLen; i ++)
				{
					Log_Printf("%#x\n", g_RecvBuffer[i]);
				}
			}

		}
//
	}
	
	return 0;
}
*/
