/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2013. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package mediatek.android.IoTManager;

import mediatek.android.IoTManager.ClientInfo;

public class IoTManagerNative {
	static {
        System.loadLibrary("iotjni");
    }
	
    public IoTManagerNative()
    {
    
    }
	/*
	 * Init SmartConnection
	 */
    public native int InitSmartConnection();
    /**
     * Start SmartConnection with Home AP
     * 
     * @SSID : SSID of Home AP
	 * @Password : Password of Home AP
	 * @Auth : Auth of Home AP
     */
    public native int StartSmartConnection(String SSID, String Password, 
    	String Target, byte Auth);
	
	/**
     * Stop SmartConnection by user
     * 
     */
	
	public native int StopSmartConnection();
	
	/*
	 * initization Control Server
	 * 
	 * return : 0 : success
	 *          1 : failed
	 */
	public native int InitControlServer(String IPAddr, String MACAddr, int ServType);
    
	/*
	 * Query all client info of connected device 
	 *
	 */
	 
	public native ClientInfo[] QueryClientInfo(int ServType);
	
	public native int CtrlClientOffline(int ClientID);
	
	public native int SetGPIO(int ClientID, int GPIOList, int GPIOValue);

	public native int[] GetGPIO(int ClientID);

	public native int SetUARTData(int ClientID, String pData, int Len);
	
	public native String GetUARTData(int ClientID);
	
	public native int SetPWM(int ClientID, short Red, short Green, short Blue);
	
	public native int[] GetPWM(int ClientID);

	public native int SetCtrlPassword(String pPassword);
	
	public native int AddFriend(String pFriendID);

	public native int InitCtrlPassword(String pPassword);

	public native int SetDataEncrypt(int iDataEncrypt);
}