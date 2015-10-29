#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <errno.h>
#include "crypt_aes.h"
#include <netinet/if_ether.h>

#include "IoTControlProtocol.h"
#include "hwtest_log.h"

#define IoT_CONTROL_PORT      7681
#define DEVICE_DISCOVERY_PORT 7682
#define BROADCAST_IP_ADDR "255.255.255.255"
#define MUTICAST_IP_ADDR "234.1.1.1"
#define INADDR_TEST "192.168.1.156"

#define DEVICE_DISCOVERY_TIMEOUT_SEC 3 
#define REQ_RESP_TIMEOUT_SEC         3 
#define REQ_RESP_TIMEOUT_USEC        40000

#define CONTROL_USE_BROADCAST 0
#define CONTROL_USE_MUTICAST  1
#define CONTROL_USE_UNICAST   2
#define SOCKTYPE_TCP          0
#define SOCKTYPE_UDP          1

#define PACKAGE_SEND_TIMES    3
#define FALSE               0
#define TRUE                1

#define NETWORK_LAN			0
#define NETWORK_WLAN        1

#define Log_Printf HWTEST_LOGD

typedef char BYTE;


typedef struct t_ClientSock
{
	/*receive and send buffer*/
//	UINT_8 RecvBuffer[PACKAGE_BUFFER_MAX_LEN];
//	UINT_8 SendBuffer[PACKAGE_BUFFER_MAX_LEN];

	UINT_16 SequenceNum;
	/*create by control password */
	UINT_32 SessionID;
	
	/*socket related information*/
	UINT_32 Socket;
	struct sockaddr_in Addr;
}ClientSock;

typedef struct t_ClientInfo
{
	struct t_ClientInfo *Next;
	struct t_ClientInfo *Prev;

	/*uplayer will use clientid to identify each client*/
	UINT_32 ClientID;
	
	/*client mac address and ip address*/
	UINT_8 ClientMACAddr[MAC_ADDR_LEN];
	UINT_8 ClientIPAddr[IP_ADDR_LEN];

	/*client capability get by IoT device*/
	ClientCapability ClientCapab;

	UINT_16 SequenceNum;
	/*create by control password */
	UINT_32 SessionID;
	
	/*socket related information*/
	UINT_32 Socket;
	struct sockaddr_in Addr;
}ClientInfo;

typedef struct t_CtrlAdpter
{
	struct sockaddr_in DeviceDisSendAddr;   //for device discovery message send
	struct sockaddr_in DeviceDisRecvAddr;   //for device discovery message recv
	UINT_32 DeviceDisSendSocket;            //for device discovery socket, UDP broadcast send socket
	UINT_32 DeviceDisRecvSocket;            //for device discovery socket, UDP broadcast recv socket
	UINT_8 RecvBuffer[PACKAGE_BUFFER_MAX_LEN];
	UINT_8 SendBuffer[PACKAGE_BUFFER_MAX_LEN];
	UINT_8 SecurityKey[AES_BLOCK_SIZES];    //AES encrypt Key
	UINT_32 ClientID;						//max client id 
	UINT_32 SessionID;						//Seesion ID, create by control password
	UINT_8 DeviceDiscoveryFlag;             //Device discovery Flag
	UINT_8 SocketType;                      //Connect socket type.0: TCP, 1: UDP
	UINT_8 OwnMACAddress[MAC_ADDR_LEN];
	ClientInfo *ClientInfoList;             //All Client information List	

	UINT_8 NetworkType;						//LAN: 0, WLAN: 1
	UINT_8 DataEncrpty; 				    //0: disable,  1:enable

	UINT_32 ControlServSocket;              //the socket connected to IoT control server
	struct sockaddr_in ControlServAddr;   //IoT control server addr structure
}CtrlAdpter;

int InitDeviceDiscoveryServer(const char *MACAddr);
int InitInternetControlServer(const char *IPAddr, const char *MACAddr);
ClientInfo *QueryClientInfo(UINT_32 *Len, UINT_32 ServType);
int CtrlClientOffline(UINT_32 ClientID);
int SetPWM(UINT_32 ClientID, UINT_16 Red, UINT_16 Green, UINT_16 Blue);
int GetPWM(UINT_32 ClientID, UINT_16 *Red, UINT_16 *Green, UINT_16 *Blue);
int SetUART(UINT_32 ClientID, const char *pData, int DataLen);
int GetUART(UINT_32 ClientID, char *pData, int *DataLen);
int SetGPIO(UINT_32 ClientID, UINT_32 GPIOList, UINT_32 GPIOValue);
int GetGPIO(UINT_32 ClientID, UINT_32 *GPIOList, UINT_32 *GPIOValue);
int InitCtrlPassword(UINT_32 iSeesionID);
int SetCtrlPassword(UINT_32 iSeesionID);
int AddFriend(const char *pFriendID);
int SetDataEncrypt(int iDataEncrypt);

