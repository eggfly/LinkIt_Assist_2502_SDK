extern "C"{
#include "../mtksrc/SmartConnection.h"
#include "../mtksrc/IoTControl.h"
}
#include "iotjni.h"

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    InitSmartConnection
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_InitSmartConnection
  (JNIEnv *, jobject)
{
    int iRst = 0;
    iRst = InitSmartConnection();
    if (iRst != 0)
    {
    	HWTEST_LOGD("StopSmartConnection error.");
    }
    return iRst;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    StartSmartConnection
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;B)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_StartSmartConnection
  (JNIEnv *env, jobject thiz, jstring nSSID, jstring nPassword, jstring nTarget, jbyte nAuth)
{
    int iRst = 0;
    const char *pSSID = NULL;
    const char *pPassword = NULL;
    const char *pTarget = NULL;

    pSSID = env->GetStringUTFChars(nSSID, 0);
    pPassword = env->GetStringUTFChars(nPassword, 0);
    pTarget = env->GetStringUTFChars(nTarget, 0);

    iRst = StartSmartConnection(pSSID, pPassword, pTarget, (char)nAuth);
    if (iRst != 0)
    {
            HWTEST_LOGD("StartSmartConnection error.");
    }
    HWTEST_LOGD("Leave JNI_StartSmartConnection.");
    return iRst;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    StopSmartConnection
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_StopSmartConnection
  (JNIEnv *, jobject)
{
    int iRst = 0;
    iRst = StopSmartConnection();
    if (iRst != 0)
    {
            HWTEST_LOGD("StopSmartConnection error.");
    }
    return iRst;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    InitControlServer
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_InitControlServer
  (JNIEnv *env, jobject thiz, jstring jIPAddr, jstring jMACAddr,jint jServType)
{
    int iRet = 0;
    const char *pIPAddr = NULL;
    const char *pMACAddr = NULL;

    pIPAddr = env->GetStringUTFChars(jIPAddr, 0);
    pMACAddr = env->GetStringUTFChars(jMACAddr, 0);

    HWTEST_LOGD("InitControlServer() type = %d. [%s]. MAC = [%s]",
                            jServType, pIPAddr, pMACAddr);

    //0 means Use broadcast to send message
    if (0 == jServType)
    {
            iRet = InitDeviceDiscoveryServer(pMACAddr);
    }
    else
    {
            iRet = InitInternetControlServer(pIPAddr, pMACAddr);
    }
    if (iRet < 0)
    {
            HWTEST_LOGD("InitControlServer() error.");
            return iRet;
    }

    return iRet;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    QueryClientInfo
 * Signature: (I)[Lmediatek/android/IoTManager/ClientInfo;
 */
JNIEXPORT jobjectArray JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_QueryClientInfo
  (JNIEnv *env, jobject thiz, jint jServType)
{
    unsigned int iClientNum = 0;
    ClientInfo *pClientList = NULL;
    HWTEST_LOGD("Enter JNI_QueryClientInfo.");

    pClientList = QueryClientInfo(&iClientNum, jServType);
    HWTEST_LOGD("QueryClientInfo : [%d] Clients", iClientNum);

    if (0 == iClientNum)
    {
            HWTEST_LOGD("QueryClientInfo : No Clients Connected");
    }

    jclass objClass = env->FindClass("mediatek/android/IoTManager/ClientInfo");
    jobjectArray ClientList = env->NewObjectArray((jsize)iClientNum, objClass, 0);

    jclass objectClass = env->FindClass("mediatek/android/IoTManager/ClientInfo");

    if (NULL == objectClass)
    {
            HWTEST_LOGD("objectClass is null");
    }
    jfieldID ClientIDFiledID = env->GetFieldID(objectClass, "ClientID", "I");
    jfieldID VendorNameFiledID = env->GetFieldID(objectClass, "VendorName", "Ljava/lang/String;");
    jfieldID ProductTypeFiledID = env->GetFieldID(objectClass, "ProductType", "Ljava/lang/String;");
    jfieldID ProductNameFiledID = env->GetFieldID(objectClass, "ProductName", "Ljava/lang/String;");
    jfieldID IPAddressFiledID = env->GetFieldID(objectClass, "IPAddress", "Ljava/lang/String;");

//      jmethodID mIDCLientID = env->GetMethodID(objectClass, "SetID", "(I)V");
//      jmethodID mIDVendorName = env->GetMethodID(objectClass, "SetVendorName", "(Ljava/lang/String;)V");
//      jmethodID mIDProductType = env->GetMethodID(objectClass, "SetProductType", "(Ljava/lang/String;)V");
//      jmethodID mIDProductName = env->GetMethodID(objectClass, "SetProductName", "(Ljava/lang/String;)V");

    jstring vendor;
    jstring product;
    jstring type;
    jstring other;
    jstring IPAddr;

    jint i = 0;
    for(i = 0; i < iClientNum; i ++)
    {
//              env->CallVoidMethod(objectClass, mIDCLientID, i);
//              env->CallVoidMethod(objectClass, mIDVendorName, vendor);
//              env->CallVoidMethod(objectClass, mIDProductType, type);
//              env->CallVoidMethod(objectClass, mIDProductName, product);

            vendor = env->NewStringUTF((const char *)(pClientList->ClientCapab.VendorName));
            product = env->NewStringUTF((const char *)(pClientList->ClientCapab.ProductName));
            type = env->NewStringUTF((const char *)(pClientList->ClientCapab.ProductType));
            IPAddr = env->NewStringUTF((const char *)(pClientList->ClientIPAddr));

            jclass oClass = env->FindClass("mediatek/android/IoTManager/ClientInfo");
            jobject Obj = env->AllocObject(oClass);
            env->SetIntField(Obj, ClientIDFiledID, pClientList->ClientID);
            env->SetObjectField(Obj, VendorNameFiledID, vendor);
            env->SetObjectField(Obj, ProductTypeFiledID, type);
            env->SetObjectField(Obj, ProductNameFiledID, product);
            env->SetObjectField(Obj, IPAddressFiledID, IPAddr);

            HWTEST_LOGD("i = %d", i);
            env->SetObjectArrayElement(ClientList, i, Obj);

            pClientList = pClientList->Next;
    }

    for (i = 0; i < env->GetArrayLength(ClientList); i ++)
    {
            jobject Obj = env->GetObjectArrayElement(ClientList, i);
            HWTEST_LOGD("Array : ClientID = %d", env->GetIntField(Obj, ClientIDFiledID));
    }

    HWTEST_LOGD("Leave JNI_QueryClientInfo.");
    return ClientList;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    CtrlClientOffline
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_CtrlClientOffline
  (JNIEnv *env, jobject thiz, jint jClientID)
{
    int iRet = 0;
    HWTEST_LOGD("Enter JNI_ControlClientOffline.");
    iRet = CtrlClientOffline(jClientID);
    if (iRet < 0)
    {
            HWTEST_LOGD("jClientID does not exist.");
    }
    HWTEST_LOGD("leavl JNI_ControlClientOffline.");

    return iRet;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    SetGPIO
 * Signature: (III)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_SetGPIO
  (JNIEnv *env, jobject thiz, jint jClientID, jint GPIOList, jint GPIOValue)
{
    int iRst = 0;

    iRst = SetGPIO(jClientID, GPIOList, GPIOValue);

    return iRst;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    GetGPIO
 * Signature: (I)[I
 */
JNIEXPORT jintArray JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_GetGPIO
  (JNIEnv *env, jobject thiz, jint jClientID)
{
    unsigned int GPIOList = 0;
    unsigned int GPIOValue = 0;
    int iRet = 0;
    int iArrayCount = 2;
    int GPIOInfo[2] = {0};

    jintArray GPIOResult = env->NewIntArray(iArrayCount);

    HWTEST_LOGD("Enter JNI_GetGPIO.");
    iRet = GetGPIO(jClientID, &GPIOList, &GPIOValue);
//      GPIOList = 250;
//      GPIOValue = 224;

    GPIOInfo[0] = GPIOList;
    GPIOInfo[1] = GPIOValue;

    if (iRet < 0)
    {
            HWTEST_LOGD("jClientID does not exist.");
    }
    HWTEST_LOGD("leavl JNI_GetGPIO.");

//      env->SetIntArrayElement(GPIOResult, 0, (int)iGPIOMap);
//      env->SetIntArrayElement(GPIOResult, 1, GPIONumber);
    env->SetIntArrayRegion(GPIOResult, 0, 2, GPIOInfo);

    return GPIOResult;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    SetUARTData
 * Signature: (ILjava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_SetUARTData
  (JNIEnv *env, jobject thiz, jint jClientID, jstring jUartTxData, jint jLength)
{
    const char *pTxData = NULL;
    int iRet = 0;

    pTxData = env->GetStringUTFChars(jUartTxData, 0);

    iRet = SetUART(jClientID, pTxData, jLength);
    HWTEST_LOGD("UartTxData = %s \n", pTxData);

    return 0;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    GetUARTData
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_GetUARTData
  (JNIEnv *env, jobject thiz, jint jClientID)
{
    char UartRxData[512] = {};

    int iRet = 0;
    int iLen = 0;
    iRet = GetUART(jClientID, UartRxData, &iLen);
    if (iRet < 0)
    {
            HWTEST_LOGD("jClientID does not exist.");
    }

    return env->NewStringUTF(UartRxData);
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    SetPWM
 * Signature: (ISSS)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_SetPWM
  (JNIEnv *env, jobject thiz, jint jClientID, jshort jRed, jshort jGreen, jshort jBlue)
{
    int iRet = 0;
    HWTEST_LOGD("Enter JNI_SetPWM.");
    iRet = SetPWM(jClientID, jRed, jGreen, jBlue);
    if (iRet < 0)
    {
            HWTEST_LOGD("jClientID does not exist.");
    }
    HWTEST_LOGD("leavl JNI_SetPWM.");
    return iRet;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    GetPWM
 * Signature: (I)[I
 */
JNIEXPORT jintArray JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_GetPWM
  (JNIEnv *env, jobject thiz, jint jClientID)
{
    unsigned short int sGreen = 0;
    unsigned short int sRed = 0;
    unsigned short int sBlue = 0;
    int iRet = 0;
    int iArrayCount = 3;
    int PWMInfo[3] = {0};

    jintArray PWMResult = env->NewIntArray(iArrayCount);

    HWTEST_LOGD("Enter JNI_GetPWM.");

    iRet = GetPWM(jClientID, &sRed, &sGreen, &sBlue);

    PWMInfo[0] = sRed;
    PWMInfo[1] = sGreen;
    PWMInfo[2] = sBlue;

    if (iRet < 0)
    {
            HWTEST_LOGD("jClientID does not exist.");
    }
    HWTEST_LOGD("leavl JNI_GetPWM.");

//      env->SetIntArrayElement(GPIOResult, 0, (int)iGPIOMap);
//      env->SetIntArrayElement(GPIOResult, 1, GPIONumber);
    env->SetIntArrayRegion(PWMResult, 0, 3, PWMInfo);

    return PWMResult;
}

static int PasswordtoSeesionID(const char *pPassword)
{
        int iSessionID = 0xFFFFFFFF;

        if (NULL == pPassword)
        {
                return iSessionID;
        }

        if (0 == strlen(pPassword)|| strlen(pPassword) < 4)
        {
                HWTEST_LOGD("Use default Control Password");
                return iSessionID;
        }

        memcpy((char *)&iSessionID, pPassword, 4);
        HWTEST_LOGD("Session ID = 0x%x", iSessionID);

        return iSessionID;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    SetCtrlPassword
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_SetCtrlPassword
  (JNIEnv *env, jobject thiz, jstring jPassword)
{
    int iRet = 0;
    const char *pPassword = NULL;
    int iSessionID = 0xFFFFFFFF;

    HWTEST_LOGD("Enter JNI_SetCtrlPassword");

    pPassword = env->GetStringUTFChars(jPassword, 0);
    iSessionID = PasswordtoSeesionID(pPassword);
    HWTEST_LOGD("Set Control Password = [%s]", pPassword);

    iRet = SetCtrlPassword(iSessionID);
    if (iRet < 0)
    {
            HWTEST_LOGD("Control password set error.");
    }
    HWTEST_LOGD("leavl JNI_SetCtrlPassword.");
    return iRet;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    AddFriend
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_AddFriend
  (JNIEnv *env, jobject thiz, jstring jFriendID)
{
    int iRet = 0;
    const char *FriendID = NULL;
//      int iSessionID = 0xFFFFFFFF;

    HWTEST_LOGD("Enter JNI_AddFriend");

    FriendID = env->GetStringUTFChars(jFriendID, 0);
    HWTEST_LOGD("Init Control Password = [%s]", FriendID);

    iRet = AddFriend(FriendID);
    if (iRet < 0)
    {
            HWTEST_LOGD("Add Friend error.");
    }
    HWTEST_LOGD("leavl JNI_AddFriend.");
    return iRet;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    InitCtrlPassword
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_InitCtrlPassword
  (JNIEnv *env, jobject thiz, jstring jPassword)
{
    int iRet = 0;
    const char *pPassword = NULL;
    int iSessionID = 0xFFFFFFFF;

    HWTEST_LOGD("Enter JNI_InitCtrlPassword");

    pPassword = env->GetStringUTFChars(jPassword, 0);
    HWTEST_LOGD("Init Control Password = [%s]", pPassword);
    iSessionID = PasswordtoSeesionID(pPassword);

    iRet = InitCtrlPassword(iSessionID);
    if (iRet < 0)
    {
            HWTEST_LOGD("Control password set error.");
    }
    HWTEST_LOGD("leavl JNI_InitCtrlPassword.");
    return iRet;
}

/*
 * Class:     mediatek_android_IoTManager_IoTManagerNative
 * Method:    SetDataEncrypt
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_mediatek_android_IoTManager_IoTManagerNative_SetDataEncrypt
  (JNIEnv *env, jobject thiz, jint jDataEncrypt)
{
    int iRet = 0;
    HWTEST_LOGD("Enter JNI_SetDataEncrypt.");

    SetDataEncrypt(jDataEncrypt);

    HWTEST_LOGD("leavl JNI_SetDataEncrypt.");

    return iRet;
}
