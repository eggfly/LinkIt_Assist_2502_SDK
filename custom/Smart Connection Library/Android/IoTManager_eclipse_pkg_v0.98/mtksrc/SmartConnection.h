#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "hwtest_log.h"

#define MCAST_PORT 8888
#define MCAST_DATA "MUTICAST TEST DATA"            

/*type define*/
typedef int MUTICAST_IP;
typedef char BYTE;

#define Log_Print HWTEST_LOGD

/*smart connection protocol related*/
#define SYNC_TYPE 0
#define DATA_TYPE 1

#define SYNC_SEND_TIMES 3
#define DATA_SEND_TIMES 1

#define SYNC_PACKAGE_SIZE  64//8
#define DATA_PACKAGE_SIZE  256//64
#define SIZE_INCREASE      128//16

#define MAC_ADDR_LEN 6

#define MUTIIP_PAD_BYTE         0x00
#define MUTIIP_FIRST_BYTE       0xEA  //234.*.*.*
#define MUTIIP_SYNC_BYTE_ONE    0x12
#define MUTIIP_SYNC_BYTE_TWO    0x13
#define MUTIIP_SYNC_BYTE_THREE  0x14
#define MUTIIP_DATA_INDEX_INIT  0x15

//TLV len define
#define WPAWPA2_PMK_LEN			32

typedef struct t_MuticastList
{
	struct t_MuticastList *Next;
	MUTICAST_IP *pMuticastList;
	int iCount;                 //Number of Muticast IP address in List
	int iMaxCount;              //Max Number of Muticast IP in List
	short int iType;			//0: sync package, 1: data package
	short int iTimes;			//Times of Muticast IP List send
} MuticastIPList;

typedef struct t_SmartConnectionParm
{
	char SSID[32]; //SSID
	char Password[64]; //Password
	char Target[MAC_ADDR_LEN];  //target mac address
	char AuthMode;   //AuthMode
}SmartConnectionParm;

typedef struct t_SmartConnectionData
{
	unsigned char Type;
	unsigned char Len;
	char *Value;
}SmartConnectionData;

typedef enum t_ConnectionDataType
{
	WPAWPA2PMK = 1,
	TYPEMAX
}ConnectionDataType;

int StartSmartConnection(const char *SSID, const char *Password, 
								const char *Target, BYTE AuthMode);
int StopSmartConnection(void);
int InitSmartConnection(void);
int StartBroadCastSSID(BYTE *SSID);
int StopBroadCastSSID(void);
