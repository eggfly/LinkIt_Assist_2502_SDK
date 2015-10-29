#include "IoTControlProtocol.h"
#define Log_Printf printf


//extern UINT_32 g_SessionID;
//extern CtrlAdpter g_CtrlAdpter;
/*
 * 
 * 
 * return : package len of buffer
 * 
 */
UINT_32 PackageHeaderBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, UINT_16 SequenceNum,
					UINT_32 SessionID, CommandType Type, UINT_8 SubType)
{
	UINT_32 PackLen = 0;
	IoTCtrlProtocolHeader *CPHeader;
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	memset(Buffer, 0, PACKAGE_BUFFER_MAX_LEN);

	CPHeader = (IoTCtrlProtocolHeader *)Buffer;
	CPHeader->Magic = IOT_MAGIC_NUMBER;
	memcpy(CPHeader->ReceiveMAC, ReceiveMAC, MAC_ADDR_LEN);
	memcpy(CPHeader->SendMAC, SendMAC, MAC_ADDR_LEN);
	CPHeader->SessionID = SessionID;
	CPHeader->Sequence = SequenceNum;
	CPHeader->Flag = 0x0;
	CPHeader->Type = Type;
	CPHeader->SubType = SubType;
	
	PackLen =  sizeof(IoTCtrlProtocolHeader);
	
	return PackLen;
}

/*Management command package build*/
UINT_32 QueryClientCapabilityRequestBuild(void *Buffer, UINT_8 *ReceiveMAC, 
						UINT_8 *SendMAC, UINT_16 SequenceNum, UINT_32 SessionID)
{
	UINT_32 HeaderLen = 0;
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	HeaderLen = PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC,
					SequenceNum, SessionID, MANAGEMENT, QUERY_CAPAB_REQUEST);

	return HeaderLen;
}

UINT_32 QueryClientCapabilityResponseBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
					UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 Len)
{
	DataHeader *pDHeader = NULL;
	UINT_32 DataLen = 0;
	UINT_32 HeaderLen = 0;
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	HeaderLen = PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, 
				SequenceNum, SessionID, MANAGEMENT, QUERY_CAPAB_RESPONSE);
	
	pDHeader = (DataHeader *)((UINT_8 *)Buffer + HeaderLen);
	pDHeader->Type = CLIENT_CAPABILITY;
	pDHeader->Length = Len;
	memcpy((UINT_8 *)Buffer + HeaderLen + sizeof(DataHeader), Data, Len);
	
	DataLen = sizeof(DataHeader) + Len;
	
	return (DataLen + HeaderLen);
}

UINT_32 ControlClientOfflineRequestBuild(void *Buffer, UINT_8 *ReceiveMAC, 
	UINT_8 *SendMAC, UINT_16 SequenceNum, UINT_32 SessionID)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID, 
						MANAGEMENT, CONTROL_CLIENT_OFFLINE_REQUEST);
}

UINT_32 ControlClientOfflineResponseBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
			UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_16 Len)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
						MANAGEMENT, CONTROL_CLIENT_OFFLINE_RESPONSE);
}

UINT_32 ControlPasswordSetRequestBuild(void *Buffer, UINT_8 *ReceiveMAC, 
		UINT_8 *SendMAC, UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 Len)
{
	DataHeader *pDHeader = NULL;
	UINT_32 DataLen = 0;
	UINT_32 HeaderLen = 0;
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
//	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, 
//						MANAGEMENT, CONTROL_PASSWORD_SET_REQUEST);

	HeaderLen = PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum,SessionID, 
								   MANAGEMENT, CONTROL_PASSWORD_SET_REQUEST);
	
	pDHeader = (DataHeader *)((UINT_8 *)Buffer + HeaderLen);
	pDHeader->Type = CONTROL_PASSWORD;
	pDHeader->Length = Len;
	memcpy((UINT_8 *)Buffer + HeaderLen + sizeof(DataHeader), Data, Len);
	
	DataLen = sizeof(DataHeader) + Len;

	return (DataLen + HeaderLen);
}

UINT_32 ControlPasswordSetConfirmBuild(void *Buffer, UINT_8 *ReceiveMAC, 
				UINT_8 *SendMAC, UINT_16 SequenceNum, UINT_32 SessionID)
{
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
								   MANAGEMENT, CONTROL_PASSWORD_SET_CONFIRM);
	
}
/*Function command package build*/
//GPIO
UINT_32 SetGPIORequestBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
				UINT_16 SequenceNum, UINT_32 SessionID, UINT_32 GPIOList, UINT_32 GPIOValue)
{
	DataHeader *pDHeader = NULL;
	UINT_32 DataLen = 0;
	UINT_32 HeaderLen = 0;
	GPIO_Information *pGPIOData = NULL;
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	HeaderLen = PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
									FUNCTION, GPIO_SET_REQUEST);
	
	pDHeader = (DataHeader *)((UINT_8 *)Buffer + HeaderLen);
	pDHeader->Type = GPIO_INFORMATION;
	pDHeader->Length = sizeof(GPIO_Information);
	
	pGPIOData = (GPIO_Information *)((UINT_8 *)Buffer + HeaderLen + sizeof(DataHeader));
	pGPIOData->GPIO_List = GPIOList;
	pGPIOData->GPIO_Value = GPIOValue;
	
	DataLen = sizeof(DataHeader) + sizeof(GPIO_Information);
	
	return (DataLen + HeaderLen);
}

UINT_32 SetGPIOResponseBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
		UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 Status, UINT_16 DataLen)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
							  FUNCTION, GPIO_SET_RESPONSE);
}

UINT_32 GetGPIORequestBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
							UINT_16 SequenceNum, UINT_32 SessionID)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
							  FUNCTION, GPIO_GET_REQUEST);
}

UINT_32 GetGPIOResponseBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
			UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 DataLen)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
							  FUNCTION, GPIO_GET_RESPONSE);
}

//UART
UINT_32 SetUARTRequestBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
			UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 Len)
{
	DataHeader *pDHeader = NULL;
	UINT_32 DataLen = 0;
	UINT_32 HeaderLen = 0;
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	HeaderLen = PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
								   FUNCTION, UART_SET_REQUEST);
	
	pDHeader = (DataHeader *)((UINT_8 *)Buffer + HeaderLen);
	pDHeader->Type = UART_INFORMATION;
	pDHeader->Length = Len;
	memcpy((UINT_8 *)Buffer + HeaderLen + sizeof(DataHeader), Data, Len);
	
	DataLen = sizeof(DataHeader) + Len;
	
	return (DataLen + HeaderLen);
}

UINT_32 SetUARTResponseBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
			UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 DataLen)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
						 	  FUNCTION, UART_SET_RESPONSE);
}

UINT_32 GetUARTRequestBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
						UINT_16 SequenceNum, UINT_32 SessionID)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
							  FUNCTION, UART_GET_REQUEST);
}

UINT_32 GetUARTResponseBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
						UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 DataLen)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
							  FUNCTION, UART_GET_RESPONSE);
}

//PWM
UINT_32 SetPWMRequestBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
					UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 Len)
{
	DataHeader *pDHeader = NULL;
	UINT_32 DataLen = 0;
	UINT_32 HeaderLen = 0;
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	HeaderLen = PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
								   FUNCTION, PWM_SET_REQUEST);
	
	pDHeader = (DataHeader *)((UINT_8 *)Buffer + HeaderLen);
	pDHeader->Type = PWM_INFORMATION;
	pDHeader->Length = Len;
	memcpy((UINT_8 *)Buffer + HeaderLen + sizeof(DataHeader), Data, Len);
	
	DataLen = sizeof(DataHeader) + Len;
	
	return (DataLen + HeaderLen);
}

UINT_32 SetPWMResponseBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
				UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 DataLen)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
							  FUNCTION, PWM_SET_RESPONSE);
}

UINT_32 GetPWMRequestBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
				UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 DataLen)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
							  FUNCTION, PWM_GET_REQUEST);
}

UINT_32 GetPWMResponseBuild(void *Buffer, UINT_8 *ReceiveMAC, UINT_8 *SendMAC, 
			UINT_16 SequenceNum, UINT_32 SessionID, UINT_8 *Data, UINT_32 DataLen)
{
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	return PackageHeaderBuild(Buffer, ReceiveMAC, SendMAC, SequenceNum, SessionID,
							  FUNCTION, PWM_GET_RESPONSE);
}

UINT_32 PackageParse(void *Buffer)
{
	UINT_8 Ret = 0;
	
	if (NULL == Buffer)
	{
		Log_Printf("Buffer is NULL at %s", __func__);
	}
	
	IoTCtrlProtocolHeader *CPHeader;
	
	CPHeader = (IoTCtrlProtocolHeader *)Buffer;
	
	return Ret;
}