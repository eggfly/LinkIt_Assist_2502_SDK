#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

/*type define*/
typedef unsigned int UINT_32;
typedef unsigned short UINT_16;
typedef unsigned char UINT_8;
typedef char TYPE;

#define MAC_ADDR_LEN 6
#define IP_ADDR_LEN 16
#define DESC_MAX_LEN 32
#define UART_MAX_DATALEN 1024
#define PASSWORD_MAX_LEN 4
#define PACKAGE_BUFFER_MAX_LEN 1600
#define IOT_MAGIC_NUMBER 0x76814350
#define DEFAULT_SESSION_ID 0xFFFFFFFF

/*protocol related define*/

typedef struct t_IoTCtrlProtocolHeader
{
	UINT_32 Magic;                    //protocol magic number
	UINT_8 ReceiveMAC[MAC_ADDR_LEN];  //receive mac address
	UINT_8 SendMAC[MAC_ADDR_LEN];     //sender mac address
	UINT_32 SessionID;                //create by control password, default is 0xFFFFFFFF
	UINT_16 Sequence;                 //sequence number
	UINT_16 Flag:4;                   //reserved
	UINT_16 Type:4;                   //command type. 0: management command 1: function command
	UINT_16 SubType:8;	              //subtype
} IoTCtrlProtocolHeader;

typedef enum t_CommandType
{
	MANAGEMENT,
	FUNCTION,
	TYPE_MAX
}CommandType;

typedef enum t_ManagementCommand
{
	QUERY_CAPAB_REQUEST = 1,
	QUERY_CAPAB_RESPONSE,
	CONTROL_CLIENT_OFFLINE_REQUEST,
	CONTROL_CLIENT_OFFLINE_RESPONSE,
	CONTROL_PASSWORD_SET_REQUEST,
	CONTROL_PASSWORD_SET_RESPONSE,
	CONTROL_PASSWORD_SET_CONFIRM,
	MANAGEMENT_COMMAND_MAX
}ManagementCommand;

typedef enum t_FunctionCommand
{
	GPIO_SET_REQUEST = 1,
	GPIO_SET_RESPONSE,
	GPIO_GET_REQUEST,
	GPIO_GET_RESPONSE,
	UART_SET_REQUEST,
	UART_SET_RESPONSE,
	UART_GET_REQUEST,
	UART_GET_RESPONSE,
	PWM_SET_REQUEST,
	PWM_SET_RESPONSE,
	PWM_GET_REQUEST,
	PWM_GET_RESPONSE,
	FUNCTION_COMMAND_MAX
}FunctionCommand;

typedef enum t_DataType
{
	STATUS,
	CLIENT_CAPABILITY,
	UART_INFORMATION,
	GPIO_INFORMATION,
	PWM_INFORMATION,
	CONTROL_PASSWORD
}DataType;

/*Control Data information*/
typedef struct t_DataHeader
{
	UINT_16 Type;
	UINT_16 Length;
}DataHeader;

typedef struct t_Status
{
	UINT_8 StatusCode;
}Status;

typedef struct t_UART_Information
{
	UINT_8 Data[UART_MAX_DATALEN];
}UART_Information;

typedef struct t_Control_Password
{
	UINT_8 Password[PASSWORD_MAX_LEN];
}Control_Password;

typedef struct t_GPIO_Information
{
	UINT_32 GPIO_List;
	UINT_32 GPIO_Value;
}GPIO_Information;

typedef struct t_PWM_Information
{
	UINT_16 RedLevel;
	UINT_16 GreenLevel;
	UINT_16 BlueLevel;
}PWM_Information;

typedef struct t_ClientCapability
{
	UINT_16 VendorNameLen;
	UINT_8 VendorName[DESC_MAX_LEN];
	UINT_16 ProductTypeLen;
	UINT_8 ProductType[DESC_MAX_LEN];
	UINT_16 ProductNameLen;
	UINT_8 ProductName[DESC_MAX_LEN];
}ClientCapability;