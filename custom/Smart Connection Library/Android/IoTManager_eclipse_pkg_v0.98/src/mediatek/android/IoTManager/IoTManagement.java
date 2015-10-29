package mediatek.android.IoTManager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ListActivity;
import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

public class IoTManagement extends ListActivity {
	
	private ClientInfo[] ClientInfoList = null;
	private IoTManagerNative IoTManager;
	private String TAG = "IoTManagement";
	private ListView mListViewClient;
	lvButtonAdapter ListView_ClientsAdaper;
	ListClients mListClients;
	public int mButtonOnFlag = 0;
	private Button mQueryClientButton;
	private Button mCtrlPasswordSetButton;
	private Button mAddFriendButton;
	private Button mServInitButton;
	private EditText mEditIPAddr;
//	private EditText mEditPort;
	private EditText mEditCtrlPassword;
	private EditText mEditFriendID;
	lvButtonAdapter ListView_ClientsAdape;
	private Spinner NetworkTypeSpinner = null;
	int mNetworkType = 1;                    //serv type: 0: LAN , 1 WAN
	private String mMACAddr;
	private WifiManager mWifiManager;
	private CheckBox mCheckDataEncrypt;
	private Dialog mServInitAlertDialog;
	
	private ArrayAdapter<String>	mNetworkTypeAdapter = null;

	final  String[] mNetworkName = {
            "LAN", "WAN"
            };
	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		System.exit(0);
		super.onDestroy();
	}
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.management);
		
		Log.d(TAG, "On Create");
		IoTManager = new IoTManagerNative();
		mServInitAlertDialog = new AlertDialog.Builder(this).
								setTitle("Error").
								setMessage("Init Server Error!").
								create();

		mWifiManager = (WifiManager) getSystemService (Context.WIFI_SERVICE); 
		if(mWifiManager.isWifiEnabled()){
        	WifiInfo WifiInfo = mWifiManager.getConnectionInfo();
			mMACAddr = WifiInfo.getMacAddress();
			Log.d(TAG, "MAC = " + mMACAddr);
		}
/*
		ClientInfoList = IoTManager.QueryClientInfo();
	
		for (int i = 0; i < ClientInfoList.length; i++)
		{
			Log.d(TAG, "ClientID = " + ClientInfoList[i].ClientID);
			Log.d(TAG, "1 = " + ClientInfoList[i].VendorName);
			Log.d(TAG, "2 = " + ClientInfoList[i].ProductType);
			Log.d(TAG, "3 = " + ClientInfoList[i].ProductName);
		}
*/
		mListClients = new ListClients();
//		mListClients.UpdataList(ClientInfoList);
		ListView_ClientsAdaper = new lvButtonAdapter(
										this, 
										mListClients.getData(), 
										R.layout.clientlist, 
										new String[]{"id", 
													"vendor", 
													"product", 
													"type", 													
													"img", 
													"offlinebutton", 
													"gpiobutton", 
													"letbutton", 
													"letimg", 
													"letbuttonflag", //for let button switch
													"gpiolist",      //for show gpio list
													"gpiogetvalues", //for show gpio value of gpio list
													"gpiosetvalues",
													"gpiosetbutton",
													"uartrxbutton",
													"uartrxvalues",
													"uarttxbutton",
													"uarttxset",
													"ipaddr"
													}, 
										new int[] { R.id.ClientID, 
													R.id.vendor,  
													R.id.product, 
													R.id.producttype, 
													R.id.img , 
													R.id.ClientOffLine, 
													R.id.GPIO_Query, 
													R.id.LetOnOff, 
													R.id.LetImg, 
													R.id.ClientID, //only for let button switch,  not use ClientID
													R.id.GPIO_List,
													R.id.GPIO_Values,
													R.id.Edit_GPIO_Values,
													R.id.GPIO_Set,
													R.id.Uart_Rx,
													R.id.View_Uart_Rx,
													R.id.Uart_Tx,
													R.id.Edit_Uart_Tx,
													R.id.IPaddress,
													});
								
		mListViewClient = (ListView) findViewById(android.R.id.list);
		mListViewClient.setAdapter(ListView_ClientsAdaper);

		mEditIPAddr = (EditText)findViewById(R.id.IPAddrEdit);
		mEditFriendID = (EditText)findViewById(R.id.FriendIDEdit);
		mEditIPAddr.setEnabled(false);
		mEditFriendID.setEnabled(false);
		
		mServInitButton = (Button)findViewById(R.id.InitServerButton);
		mServInitButton.setOnClickListener(mIniTServListener);
		
		mQueryClientButton = (Button)findViewById(R.id.QueryClientList);
		mQueryClientButton.setOnClickListener(mQueryClientListener);
		mQueryClientButton.setEnabled(false);
		
		mCtrlPasswordSetButton = (Button)findViewById(R.id.SetCtrlPassword);
		mCtrlPasswordSetButton.setOnClickListener(mCtrlPasswordSetListener);
		mCtrlPasswordSetButton.setEnabled(false);

		mAddFriendButton = (Button)findViewById(R.id.AddFriend);
		mAddFriendButton.setOnClickListener(mAddFriendSetListener);
		mAddFriendButton.setEnabled(false);
			
		NetworkTypeSpinner = (Spinner) findViewById(R.id.NetWork_Type_Spinner);
	    mNetworkTypeAdapter = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item, mNetworkName);
	    mNetworkTypeAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);	
	    NetworkTypeSpinner.setAdapter(mNetworkTypeAdapter);
		
		mCheckDataEncrypt= (CheckBox)findViewById(R.id.EncryptCheck);
		mCheckDataEncrypt.setOnCheckedChangeListener(mDataEncryptCBListener);

		NetworkTypeSpinner.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {

			public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3)
			{				
				mNetworkType = arg2;
				if (0 == mNetworkType) //LAN
				{
					mEditIPAddr.setEnabled(false);
					mEditFriendID.setEnabled(false);
//					mCtrlPasswordSetButton.setEnabled(true);
					mAddFriendButton.setEnabled(false);
					mAddFriendButton.setEnabled(false);

				}
				else                   //WAN
				{
					mEditIPAddr.setEnabled(true);
					mEditFriendID.setEnabled(true);
					mCtrlPasswordSetButton.setEnabled(true);
					mAddFriendButton.setEnabled(true);
				}
				arg0.setVisibility(View.VISIBLE);
			}

			public void onNothingSelected(AdapterView<?> arg0)
			{
				// TODO Auto-generated method stub
			}

		});
		
		ListView_ClientsAdaper.notifyDataSetChanged();
	}	
	
	Button.OnClickListener mQueryClientListener = new Button.OnClickListener() {

		public void onClick(View arg0) {			
			UpdateClientInfo();
		}
	};
	
	CheckBox.OnCheckedChangeListener mDataEncryptCBListener = new CheckBox.OnCheckedChangeListener(){
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
		{
			if (isChecked)
			{
				IoTManager.SetDataEncrypt(1);
			}
			else
			{
				IoTManager.SetDataEncrypt(0);
			}
		}
	};

	Button.OnClickListener mCtrlPasswordSetListener = new Button.OnClickListener() {

		public void onClick(View arg0) {
			String CtrlPassword;
			mEditCtrlPassword = (EditText)findViewById(R.id.CtrlPasswordEdit);
			
			CtrlPassword = mEditCtrlPassword.getText().toString();
			
			int iRet = IoTManager.SetCtrlPassword(CtrlPassword);
			if (iRet < 0)
			{
				Log.d(TAG, "Set control password error");
		//		System.exit(-1);
			}
		}
	};

	Button.OnClickListener mIniTServListener = new Button.OnClickListener() {
	
		public void onClick(View arg0) {			
			String IPAddr;
			String CtrlPassword;

			mEditIPAddr = (EditText)findViewById(R.id.IPAddrEdit);
			mEditCtrlPassword = (EditText)findViewById(R.id.CtrlPasswordEdit);

			IPAddr = mEditIPAddr.getText().toString();
			int iRet = IoTManager.InitControlServer(IPAddr, mMACAddr, mNetworkType);
			if (iRet < 0)
			{
				Log.d(TAG, "InitControlServer error, type = " + mNetworkType);
				
				mServInitAlertDialog.show();
				return;
		//		System.exit(-1);
			}
			
			CtrlPassword = mEditCtrlPassword.getText().toString();
			iRet = IoTManager.InitCtrlPassword(CtrlPassword);
			if (iRet < 0)
			{
				Log.d(TAG, "Set control password error");
		//		System.exit(-1);
			}

			mQueryClientButton.setEnabled(true);
			mCtrlPasswordSetButton.setEnabled(true);
			if (1 == mNetworkType)
			{
				mAddFriendButton.setEnabled(true);
			}

			mServInitButton.setEnabled(false);
		}
	};


	Button.OnClickListener mAddFriendSetListener = new Button.OnClickListener() {

		public void onClick(View arg0) {
			String FriendID;
			mEditFriendID = (EditText)findViewById(R.id.FriendIDEdit);
			
			FriendID = mEditFriendID.getText().toString();
			
			int iRet = IoTManager.AddFriend(FriendID);
			if (iRet < 0)
			{
				Log.d(TAG, "Add friend error : " + FriendID);
		//		System.exit(-1);
			}
		}
	};
	public void UpdateClientInfo()
	{
		ArrayList<HashMap<String, Object>> ClientListMap;
		ClientInfo[] ClientInfoList = null;
		int i = 0;
		
		ClientInfoList = IoTManager.QueryClientInfo(mNetworkType);
		mListClients.CreatList(ClientInfoList);
		ClientListMap = mListClients.getData();
		if (0 != ClientListMap.size())
		{	
			Log.d(TAG, "ClientListMap != NULL");
			ListView_ClientsAdaper.UpDateData(ClientListMap);		
			
			//only can change control password after the client information are queried
			mCtrlPasswordSetButton.setEnabled(true);

			//Only for UI show
			for (i = 0; i < ClientInfoList.length; i ++)
			{
				//for UI show : GPIO Info
				int[] GPIOInfo;
				int iGPIOList;
				int iGPIOValue;
				
				int CliID = ClientInfoList[i].ClientID;
				GPIOInfo = IoTManager.GetGPIO(CliID);
				String GPIOList = new String();
				String GPIOValues = new String();
				
				iGPIOList = GPIOInfo[0];
				iGPIOValue = GPIOInfo[1];
				int j = 0;
				for (j = 0; j < 32; j ++)
				{	
					if (1 == ((iGPIOList >> j) & 1))
					{
						GPIOList = GPIOList + " ";
						GPIOList = GPIOList + j;
						GPIOValues = GPIOValues + " ";
						if (1 == ((iGPIOValue >> j) & 1))
						{
							GPIOValues = GPIOValues + "1";
						}
						else
						{
							GPIOValues = GPIOValues + "0";
						}
					}
				}

				mListClients.UpdateList(CliID, "gpiolist", GPIOList);
				mListClients.UpdateList(CliID, "gpiogetvalues", GPIOValues);
			}
		}
		ListView_ClientsAdaper.UpDateData(ClientListMap);
		ListView_ClientsAdaper.notifyDataSetChanged();
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		// TODO Auto-generated method stub
		super.onListItemClick(l, v, position, id);
		l.getItemAtPosition(position);
		IoTManager.SetPWM(1, (short)5, (short)5, (short)5);
		Log.d(TAG, "Clicked: ID = " + position);
	}
	
	public class ListClients {

		ArrayList<HashMap<String, Object>> ClientListMap = new ArrayList<HashMap<String, Object>>();
		
		private void CreatList(ClientInfo[] ClientList) {
	//		HashMap<String, Object> map = new HashMap<String, Object>();
			ClientListMap = new ArrayList<HashMap<String, Object>>();
			for (int i = 0; i < ClientList.length; i++)
			{
			    HashMap<String, Object> map = new HashMap<String, Object>();
				map.put("id", ClientList[i].ClientID);
				map.put("vendor", ClientList[i].VendorName);
				map.put("product", ClientList[i].ProductName);
				map.put("ipaddr", ClientList[i].IPAddress);
				map.put("type", ClientList[i].ProductType);
				map.put("img", R.drawable.let);
				map.put("letbutton", R.drawable.yellow);
				map.put("letimg", R.drawable.let_dark);
				map.put("gpiobutton", R.drawable.yellow);
				map.put("offlinebutton", R.drawable.yellow);
				map.put("letbuttonflag", mButtonOnFlag);
				map.put("gpiolist", "0 1 2 3 4");
				map.put("gpiogetvalues", "0 0 0 0 0");
				map.put("gpiosetvalues", "");
				map.put("gpiosetbutton", R.drawable.yellow);
				map.put("uartrxbutton", R.drawable.yellow);
				map.put("uartrxvalues", "");
				map.put("uarttxbutton", R.drawable.yellow);
				map.put("uarttxset", "");
				
				ClientListMap.add(map);

				Iterator iter = map.entrySet().iterator();
				while (iter.hasNext())
				{
					Map.Entry entry = (Map.Entry) iter.next();
					Object key = entry.getKey();
					Object val = entry.getValue();
					Log.d(TAG, "IN UpdataList key = " + key + " Value = " + val);
				}
			}
		//	ListView_ClientsAdaper.notifyDataSetChanged();
		}

		private void UpdateList(int ClientID, String Keyword, Object obj) 
		{
			HashMap<String, Object> ClientInfo;
			for (int i = 0; i < ClientListMap.size(); i ++)
			{
				ClientInfo = ClientListMap.get(i);

				int CliID = (Integer)ClientInfo.get("id");
				if (CliID == ClientID)
				{
					ClientInfo.put(Keyword, obj);
					Log.d(TAG, "Updata clientid " + ClientID + " keyword = " + Keyword);
				}
			}			
		}
		
		private ArrayList<HashMap<String, Object>> getData() {
			return this.ClientListMap;
		}
	}
	
	public class lvButtonAdapter extends BaseAdapter {
	private class buttonViewHolder {
		ImageView image;
		TextView VendorName;
		TextView ProductName;
		TextView ProductType;
		TextView IPAddress;
		Button buttonOffline;
		Button buttonQueryGPIO;
		Button buttonSetGPIO;
		Button buttonLetOnOff;
		ImageView LetImage;
		TextView GPIOList;
		TextView GPIOValues;
		EditText EditGPIO;
		EditText EditUartTx;
		TextView ViewUartRx;
		Button buttonUartTx;
		Button buttonUartRx;
	}
	
	private ArrayList<HashMap<String, Object>> mClientList;
	private LayoutInflater mInflater;
	private Context mContext;
	private String[] keyString;
	private int[] valueViewID;
	private buttonViewHolder holder;
	private int ClientID; 
	private int GPIOEditIndex = -1;
	private int UartTxEditIndex = -1;
	
	public lvButtonAdapter(Context c, ArrayList<HashMap<String, Object>> ClientList, int resource, 
			String[] from, int[] to) {
		mClientList = ClientList;
		mContext = c;
		mInflater = (LayoutInflater)mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		keyString = new String[from.length];
		valueViewID = new int[to.length];
		System.arraycopy(from, 0, keyString, 0, from.length);
		System.arraycopy(to, 0, valueViewID, 0, to.length);

		HashMap<String, Object> LogCheck;
		for (int i = 0; i < mClientList.size(); i ++)
		{
			LogCheck = mClientList.get(i);
			Iterator iter = LogCheck.entrySet().iterator();
			while (iter.hasNext())
			{
				Map.Entry entry = (Map.Entry) iter.next();
				Object key = entry.getKey();
				Object val = entry.getValue();
				Log.d(TAG, "IN Adapter key = " + key + " Value = " + val);
			}
		}
	}

	public void UpDateData(ArrayList<HashMap<String, Object>> ClientList)
	{
		mClientList = ClientList;
	}
	
	@Override
	public int getCount() {
		return mClientList.size();
	}

	@Override
	public Object getItem(int position) {
		return mClientList.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	private int OfflinebuttonClicked(int position)
	{
		HashMap<String, Object> ClientInfo = mClientList.get(position);
		int iRet = 0;
		
		ClientID = (Integer)ClientInfo.get(keyString[0]);
		
		Log.d(TAG, "ClientID =  " + ClientID + " at position " + position);
		
		iRet = IoTManager.CtrlClientOffline(ClientID);
		
		return iRet;
	}

	private int PWMbuttonClicked(int position)
	{
		HashMap<String, Object> ClientInfo = mClientList.get(position);
		int mButtonOnFlag = 0;
		int PWMValue[];
		
		ClientID = (Integer)ClientInfo.get(keyString[0]);
		mButtonOnFlag = (Integer)ClientInfo.get(keyString[9]);
		Log.d(TAG, "ClientID =  " + ClientID + " at position " + position);
		
//		if (IoTManager.SetPWM(ClientID, (short)5, (short)5, (short)5))
		if (0 == mButtonOnFlag)
		{
			IoTManager.SetPWM(ClientID, (short)5, (short)5, (short)5);
		}
		else
		{
			IoTManager.SetPWM(ClientID, (short)0, (short)0, (short)0);
		}

		PWMValue = IoTManager.GetPWM(ClientID);
		if (0 == PWMValue[0] && 0 == PWMValue[0] && 0 == PWMValue[0])
		{
			mButtonOnFlag = 0;
		}
		return 0;
	}

	private String UartRxbuttonClicked(int position)
	{
		HashMap<String, Object> ClientInfo = mClientList.get(position);
		
		ClientID = (Integer)ClientInfo.get(keyString[0]);
		
		String RxData;

		RxData = IoTManager.GetUARTData(ClientID);
		Log.d(TAG, "UartRxbuttonClicked ClientID =	" + RxData);

		return RxData;
	}

	private int UartTxbuttonClicked(int position)
	{
		HashMap<String, Object> ClientInfo = mClientList.get(position);
		int iRst = 0;
		String TxValue;
		
		ClientID = (Integer)ClientInfo.get(keyString[0]);
		
		Log.d(TAG, "UartTxbuttonClicked ClientID =	" + ClientID + " at position " + position);

		TxValue = (String)ClientInfo.get(keyString[17]);
		Log.d(TAG, "UartTxbuttonClicked values = " + TxValue + 
			" Len = " + TxValue.length() + " at position " + position);
		
		iRst = IoTManager.SetUARTData(ClientID, TxValue, TxValue.length());
		
		return 0;
	}


	/*String GPIOValueList : 2 3 4 7 8 
	  *String GPIOSetValue : 0 1 1 0 1
	  *means in GPIO bitmap, bit 2,3,4,7,8 are available. bit2 value is 0, bit3 value is 1 ...
	  *this function convert the GPIO string to bitmap
	  */
	private int[] GPIOStringToBitMap(String GPIOValueList, String GPIOSetValue)
	{
		int iResult[] = new int[2];
		int i = 0;
		int iTmp = 0;
		
		String GPIOList[] = GPIOValueList.trim().split(" ");
		String GPIOValue[] = GPIOSetValue.trim().split(" ");
		iResult[0] = 0;
		iResult[1] = 0;
		
		Log.d(TAG, " GPIOList.length = " + GPIOList.length);

		if (GPIOList.length != GPIOValue.length)
		{
			return iResult;
		}

		for (i = 0; i < GPIOList.length; i ++)
		{
			iTmp = Integer.parseInt(GPIOList[i]);
			Log.d(TAG, " iTmp = " + iTmp);
			iResult[0] |= (1 << iTmp);
			if (0 != Integer.parseInt(GPIOValue[i]))
			{
				iResult[1] |= (1 << iTmp);
			}
		}
		Log.d(TAG, "GPIOStringToBitMap iResult[0] = " + iResult[0]);
		Log.d(TAG, "GPIOStringToBitMap iResult[1] = " + iResult[1]);
		
		return iResult;
	}

	private int GPIOSetbuttonClicked(int position)
	{
		HashMap<String, Object> ClientInfo = mClientList.get(position);
		String GPIOValueList;
		String GPIOSetValue;
		int iGPIO[];
		int iGPIOList;
		int iGPIOValue;
		
        if (ClientInfo != null) {
			//get the gpio string entered by user
			ClientID = (Integer)ClientInfo.get(keyString[0]);
			GPIOValueList = (String)ClientInfo.get(keyString[10]);
			GPIOSetValue = (String)ClientInfo.get(keyString[12]);			
		
			Log.d(TAG, "ClientID =  " + ClientID + " at position " + position);	
			Log.d(TAG, "GPIOValueList =  " + GPIOValueList);
			Log.d(TAG, "GPIOSetValue =  " + GPIOSetValue);
			
			if (GPIOValueList.equals("") || GPIOSetValue.equals(""))
			{
				return -1;
			}
			iGPIO = GPIOStringToBitMap(GPIOValueList, GPIOSetValue);

			iGPIOList = iGPIO[0];
			iGPIOValue = iGPIO[1];

			if (0 == iGPIOList)
			{
				return -1;
			}
			
			int iRet = IoTManager.SetGPIO(ClientID, iGPIOList, iGPIOValue);
        }		
		
		return 0;
	}		
	

	private int[] GPIOQuerybuttonClicked(int position)
	{
		HashMap<String, Object> ClientInfo = mClientList.get(position);
		int[] GPIOInfo;
		
		ClientID = (Integer)ClientInfo.get(keyString[0]);
		
		Log.d(TAG, "ClientID =  " + ClientID + " at position " + position);
		
//		if (IoTManager.SetPWM(ClientID, (short)5, (short)5, (short)5))
		GPIOInfo = IoTManager.GetGPIO(ClientID);
		
		return GPIOInfo;
	}
	
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView != null) {
            holder = (buttonViewHolder) convertView.getTag();
			holder.EditGPIO.setTag(position);
        } else {
            convertView = mInflater.inflate(R.layout.clientlist, null);
            holder = new buttonViewHolder();
            
            holder.VendorName = (TextView)convertView.findViewById(valueViewID[1]);
			holder.ProductName = (TextView)convertView.findViewById(valueViewID[2]);
			holder.ProductType = (TextView)convertView.findViewById(valueViewID[3]);
            holder.image = (ImageView)convertView.findViewById(valueViewID[4]);
				
			holder.buttonOffline = (Button)convertView.findViewById(valueViewID[5]);
			holder.buttonQueryGPIO = (Button)convertView.findViewById(valueViewID[6]);	
			holder.buttonLetOnOff = (Button)convertView.findViewById(valueViewID[7]);					
			holder.LetImage = (ImageView)convertView.findViewById(valueViewID[8]);
			holder.GPIOList = (TextView)convertView.findViewById(valueViewID[10]);
			holder.GPIOValues = (TextView)convertView.findViewById(valueViewID[11]);
			holder.EditGPIO = (EditText)convertView.findViewById(valueViewID[12]);
			holder.buttonSetGPIO = (Button)convertView.findViewById(valueViewID[13]);
			holder.buttonUartRx = (Button)convertView.findViewById(valueViewID[14]);
			holder.ViewUartRx = (TextView)convertView.findViewById(valueViewID[15]);
			holder.buttonUartTx = (Button)convertView.findViewById(valueViewID[16]);
			holder.EditUartTx = (EditText)convertView.findViewById(valueViewID[17]);

			holder.IPAddress = (TextView)convertView.findViewById(valueViewID[18]);
			
			holder.EditGPIO.setTag(position);
			convertView.setTag(holder);
        }
        Log.d(TAG, "Clicked: position = " + position);
        HashMap<String, Object> ClientInfo = mClientList.get(position);
        if (ClientInfo != null) {
			ClientID = (Integer)ClientInfo.get(keyString[0]);
			Log.d(TAG, "Clicked: ClientID = " + ClientID);
            String VendorName = (String) ClientInfo.get(keyString[1]);
			String ProductName = (String) ClientInfo.get(keyString[2]);
			String ProductType = (String) ClientInfo.get(keyString[3]);
            int mid = (Integer)ClientInfo.get(keyString[4]);
            int bid = (Integer)ClientInfo.get(keyString[5]);
			int LetID = (Integer)ClientInfo.get(keyString[8]);
			int LetBtFlag = (Integer)ClientInfo.get(keyString[9]);
			String GPIOList = (String) ClientInfo.get(keyString[10]);
			String GPIOValues = (String) ClientInfo.get(keyString[11]);
			String GPIOSetValues = (String) ClientInfo.get(keyString[12]);
			String UartRxValues = (String) ClientInfo.get(keyString[15]);
			String UartTxValues = (String) ClientInfo.get(keyString[17]);
			String IPAddr = (String) ClientInfo.get(keyString[18]);
			
            holder.VendorName.setText(VendorName);
			holder.ProductName.setText(ProductName);
			holder.ProductType.setText(ProductType);
			holder.IPAddress.setText(IPAddr);
			holder.GPIOList.setText(GPIOList);
			holder.GPIOValues.setText(GPIOValues);
			holder.ViewUartRx.setText(UartRxValues);
			//holder.EditUartTx.setText(UartTxValues);
			//holder.EditGPIO.setText(GPIOSetValues);
            holder.image.setImageDrawable(holder.image.getResources().getDrawable(mid));
//            holder.buttonLetOnOff.setImageDrawable(holder.buttonLetOnOff.getResources().getDrawable(bid));
			holder.buttonOffline.setOnClickListener(new OfflineListener(position));
			holder.buttonQueryGPIO.setOnClickListener(new QueryGPIOListener(position));
            holder.buttonLetOnOff.setOnClickListener(new LetOnOffListener(position, LetBtFlag));
			holder.LetImage.setImageDrawable(holder.LetImage.getResources().getDrawable(LetID));
			if (0 == LetBtFlag)
			{
				holder.buttonLetOnOff.setText("ON");
			}
			else
			{
				holder.buttonLetOnOff.setText("OFF");
			}

			holder.EditGPIO.addTextChangedListener(new GPIOEditTextWatcher(position, holder));
			holder.EditGPIO.setOnTouchListener(new GPIOEditListener(position));
			holder.buttonSetGPIO.setOnClickListener(new SetGPIOListener(position));

			holder.buttonUartRx.setOnClickListener(new UartRxListener(position));
			holder.buttonUartTx.setOnClickListener(new UartTxListener(position));

			holder.EditUartTx.addTextChangedListener(new UartTxEditTextWatcher(position, holder));
			holder.EditUartTx.setOnTouchListener(new UartTxEditListener(position));

			if (GPIOEditIndex != -1 && GPIOEditIndex == position)
			{
				holder.EditGPIO.requestFocus();
				Log.d(TAG, "position = index " + position);
			}
			if (UartTxEditIndex != -1 && UartTxEditIndex == position)
			{
				holder.EditUartTx.requestFocus();
				Log.d(TAG, "position = index " + position);
			}
			
        }        
        return convertView;
    }

	class GPIOEditTextWatcher implements TextWatcher {
			private buttonViewHolder mholder;
			private int position;
	
			GPIOEditTextWatcher(int pos, buttonViewHolder holder) {
				mholder = holder;
				position = pos;
			}
			
			public void onTextChanged(CharSequence text, int shart, int before, int count){
//				String GPIOValue = mholder.EditGPIO.getText().toString();
//				Log.d(TAG, "onTextChanged Set GPIO value " + GPIOValue);
			}

			public void beforeTextChanged(CharSequence s, int shart, int count, int after){
			}

			public void afterTextChanged(Editable s){
				String GPIOValue = mholder.EditGPIO.getText().toString();
				Log.d(TAG, "Set GPIO value " + GPIOValue);
				
				HashMap<String, Object> ClientInfo = mClientList.get(position);
		        if (ClientInfo != null) {
					//save the gpio string entered by user
					ClientInfo.put(keyString[12], GPIOValue);
		        }

				String GPIOValue1 = (String)ClientInfo.get(keyString[12]);
				Log.d(TAG, "In holder GPIO value " + GPIOValue1);
			}
		}


	class GPIOEditListener implements EditText.OnTouchListener {
		private int position;

		GPIOEditListener(int pos) {
			position = pos;
		}
		
		@Override
		public boolean onTouch(View view, MotionEvent event){
			if (event.getAction() == MotionEvent.ACTION_UP)
			{
				GPIOEditIndex = position;
				UartTxEditIndex = -1;
				Log.d(TAG, "GPIO EditText Touched = " + GPIOEditIndex);
			}

			return false;
		}
	}
	
	class UartTxEditTextWatcher implements TextWatcher {
				private buttonViewHolder mholder;
				private int position;
		
				UartTxEditTextWatcher(int pos, buttonViewHolder holder) {
					mholder = holder;
					position = pos;
				}
				
				public void onTextChanged(CharSequence text, int shart, int before, int count){
	//				String GPIOValue = mholder.EditGPIO.getText().toString();
	//				Log.d(TAG, "onTextChanged Set GPIO value " + GPIOValue);
				}
	
				public void beforeTextChanged(CharSequence s, int shart, int count, int after){
				}
	
				public void afterTextChanged(Editable s){
					String UartTxValue = mholder.EditUartTx.getText().toString();
					
					HashMap<String, Object> ClientInfo = mClientList.get(position);
					if (ClientInfo != null) {
						//save the gpio string entered by user
						ClientInfo.put(keyString[17], UartTxValue);
					}
	
					String GPIOValue1 = (String)ClientInfo.get(keyString[17]);
					Log.d(TAG, "In holder UartTxValue value " + GPIOValue1);
				}
			}

	class UartTxEditListener implements EditText.OnTouchListener {
			private int position;
	
			UartTxEditListener(int pos) {
				position = pos;
			}
			
			@Override
			public boolean onTouch(View view, MotionEvent event){
				if (event.getAction() == MotionEvent.ACTION_UP)
				{
					UartTxEditIndex = position;
					GPIOEditIndex = -1;
					Log.d(TAG, "GPIO EditText Touched = " + UartTxEditIndex);
				}
	
				return false;
			}
		}

	class OfflineListener implements Button.OnClickListener {
		private int position;

		OfflineListener(int pos) {
			position = pos;
		}
		
		@Override
		public void onClick(View v) {
			
			Log.d(TAG, "Offline Clicked: position = " + position);
			GPIOEditIndex = -1;
			UartTxEditIndex = -1;
			if (0 == OfflinebuttonClicked(position))
			{
				IoTManagement.this.UpdateClientInfo();
			}	
			
		}
	}
	
	class UartRxListener implements Button.OnClickListener {
		private int position;

		UartRxListener(int pos) {
			position = pos;
		}
		
		@Override
		public void onClick(View v) {
			String RxData;
			RxData = UartRxbuttonClicked(position);

			Log.d(TAG, "RxData = " + RxData);
			
			HashMap<String, Object> ClientInfo = mClientList.get(position);
			if (ClientInfo != null) {
				//save the Rx Data
				ClientInfo.put(keyString[15], RxData);
			}
			
			GPIOEditIndex = -1;
			UartTxEditIndex = -1;
			lvButtonAdapter.this.notifyDataSetChanged();
		}
	}

	class UartTxListener implements Button.OnClickListener {
		private int position;

		UartTxListener(int pos) {
			position = pos;
		}
		
		@Override
		public void onClick(View v) {				
			UartTxbuttonClicked(position);
			GPIOEditIndex = -1;
			UartTxEditIndex = -1;
			lvButtonAdapter.this.notifyDataSetChanged();
		}
	}

	class SetGPIOListener implements Button.OnClickListener {
		private int position;

		SetGPIOListener(int pos) {
			position = pos;
		}
		
		@Override
		public void onClick(View v) {
			
			GPIOSetbuttonClicked(position);
			GPIOEditIndex = -1;
			UartTxEditIndex = -1;
			lvButtonAdapter.this.notifyDataSetChanged();
		}
	}

	class QueryGPIOListener implements Button.OnClickListener {
		private int position;

		QueryGPIOListener(int pos) {
			position = pos;
		}
		
		@Override
		public void onClick(View v) {
			int vid = v.getId();
			int[] GPIOInfo;
			int iGPIOList = 0;
			int iGPIOValue = 0;
			String GPIOList = new String();
			String GPIOValues = new String();
			Log.d(TAG, "Clicked QueryGPIO botton: position = " + position);
							
			GPIOInfo = GPIOQuerybuttonClicked(position);
			
			iGPIOList = GPIOInfo[0];
			iGPIOValue = GPIOInfo[1];
			
			Log.d(TAG, "List =  " + iGPIOList + " Value = " + iGPIOValue);
			
			for (int i = 0; i < 32; i ++)
			{	
				if (1 == ((iGPIOList >> i) & 1))
				{
					GPIOList = GPIOList + " ";
					GPIOList = GPIOList + i;
					GPIOValues = GPIOValues + " ";
					if (1 == ((iGPIOValue >> i) & 1))
					{
						GPIOValues = GPIOValues + "1";
					}
					else
					{
						GPIOValues = GPIOValues + "0";
					}
				}
			}
			Log.d(TAG, "Clicked QueryGPIO botton: GPIOList = " + GPIOList);
			Log.d(TAG, "Clicked QueryGPIO botton: GPIOValues = " + GPIOValues);
			
			HashMap<String, Object> ClientInfo = mClientList.get(position);
	        if (ClientInfo != null) {				
				ClientInfo.put(keyString[10], GPIOList);
				ClientInfo.put(keyString[11], GPIOValues);
	        }   
			
			GPIOEditIndex = -1;
			UartTxEditIndex = -1;
			lvButtonAdapter.this.notifyDataSetChanged();		
			
		}
	}
	
	class LetOnOffListener implements Button.OnClickListener {
		private int position;
		private int LetButtonOnFlag;//for let button switch
		LetOnOffListener(int pos, int flag) {
			position = pos;
			LetButtonOnFlag = flag;
		}
		
		@Override
		public void onClick(View v) {
			int vid = v.getId();

			GPIOEditIndex = -1;
			UartTxEditIndex = -1;
			if (vid == holder.buttonLetOnOff.getId())
			{
				Log.d(TAG, "Clicked: position = " + position);
				if ( 0 == PWMbuttonClicked(position))
				{
					HashMap<String, Object> ClientInfo = mClientList.get(position);
				    if (ClientInfo != null) 
					{				        						
						if (0 != LetButtonOnFlag)
						{
							LetButtonOnFlag = 0;
							ClientInfo.put(keyString[9], LetButtonOnFlag);
							ClientInfo.put(keyString[8], R.drawable.let_dark);
						}
						else 
						{
							LetButtonOnFlag = 1;
							ClientInfo.put(keyString[9], LetButtonOnFlag);
							ClientInfo.put(keyString[8], R.drawable.let_bright);
						}
						lvButtonAdapter.this.notifyDataSetChanged();
					}
				}
			}
		}
	}
	}

}
