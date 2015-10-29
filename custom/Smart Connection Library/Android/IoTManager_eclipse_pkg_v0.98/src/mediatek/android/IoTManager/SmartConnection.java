package mediatek.android.IoTManager;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.util.List;

public class SmartConnection extends Activity {
	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		System.exit(0);
		super.onDestroy();
	}
	private byte AuthModeOpen = 0x00;
	private byte AuthModeShared = 0x01;
	private byte AuthModeAutoSwitch = 0x02;
	private byte AuthModeWPA = 0x03;
	private byte AuthModeWPAPSK = 0x04;
	private byte AuthModeWPANone = 0x05;
	private byte AuthModeWPA2 = 0x06;
	private byte AuthModeWPA2PSK = 0x07;   
	private byte AuthModeWPA1WPA2 = 0x08;
	private byte AuthModeWPA1PSKWPA2PSK = 0x09;
	
	private static final String TAG = "SmartConnection";
	private Button mButtonStart; 
	private Button mButtonStop; 
	private EditText mEditSSID;
	private EditText mEditPassword;
	private TextView mTargetEdit;
	private TextView mAuthModeView;
	private CheckBox mCheckBox;
	private IoTManagerNative IoTManager;
	private int DEFAULT_WAIT_TIME = 1000;
	private WifiManager mWifiManager;
	private String mConnectedSsid;
	private String mConnectedPassword;
	private String mAuthString;
	private byte mAuthMode;

	private ProgressBar mProgressBar;
	private ProgressDialog mProgressDialog;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.smartconnection);
		
		mButtonStart = (Button)findViewById(R.id.StartButton);
		mButtonStop = (Button)findViewById(R.id.StopButton);
		mEditSSID = (EditText)findViewById(R.id.SSIDEdit);
		mEditPassword = (EditText)findViewById(R.id.PasswordEdit);
		mAuthModeView = (TextView)findViewById(R.id.AuthModeView);
		mTargetEdit = (EditText)findViewById(R.id.TargetEdit);
		mCheckBox= (CheckBox)findViewById(R.id.PwdCheck);
		mProgressBar = (ProgressBar)findViewById(R.id.loadProgressBar);
		mProgressBar.setVisibility(View.INVISIBLE);
		
		IoTManager = new IoTManagerNative();
		String Password = null;

		//Get Authentication mode of AP
		mWifiManager = (WifiManager) getSystemService (Context.WIFI_SERVICE); 
		if(mWifiManager.isWifiEnabled()){
        	WifiInfo WifiInfo = mWifiManager.getConnectionInfo();
        	mConnectedSsid = WifiInfo.getSSID();
			int iLen = mConnectedSsid.length();

			if (iLen == 0)
			{
				return;
			}
			
			if (mConnectedSsid.startsWith("\"") && mConnectedSsid.endsWith("\""))
			{
				mConnectedSsid = mConnectedSsid.substring(1, iLen - 1);
			}
	//		mConnectedSsid = mConnectedSsid.replace('\"', ' ');
	//		mConnectedSsid = mConnectedSsid.trim();
			mEditSSID.setText(mConnectedSsid);
			Log.d(TAG, "SSID = " + mConnectedSsid);
			
			List<ScanResult> ScanResultlist = mWifiManager.getScanResults();
			for (int i = 0, len = ScanResultlist.size(); i < len; i++) {
				ScanResult AccessPoint = ScanResultlist.get(i);			
				
				if (AccessPoint.SSID.equals(mConnectedSsid))
				{		
					boolean WpaPsk = AccessPoint.capabilities.contains("WPA-PSK");
		        	boolean Wpa2Psk = AccessPoint.capabilities.contains("WPA2-PSK");
					boolean Wpa = AccessPoint.capabilities.contains("WPA-EAP");
		        	boolean Wpa2 = AccessPoint.capabilities.contains("WPA2-EAP");
					
					if (AccessPoint.capabilities.contains("WEP"))
					{
						mAuthString = "OPEN-WEP";
						mAuthMode = AuthModeOpen;
						break;
					}

					if (WpaPsk && Wpa2Psk)
					{
						mAuthString = "WPA-PSK WPA2-PSK";
						mAuthMode = AuthModeWPA1PSKWPA2PSK;
						break;
					}
					else if (Wpa2Psk)
					{
						mAuthString = "WPA2-PSK";
						mAuthMode = AuthModeWPA2PSK;
						break;
					}
					else if (WpaPsk)
					{
						mAuthString = "WPA-PSK";
						mAuthMode = AuthModeWPAPSK;
						break;
					}

					if (Wpa && Wpa2)
					{
						mAuthString = "WPA-EAP WPA2-EAP";
						mAuthMode = AuthModeWPA1WPA2;
						break;
					}
					else if (Wpa2)
					{
						mAuthString = "WPA2-EAP";
						mAuthMode = AuthModeWPA2;
						break;
					}
					else if (Wpa)
					{
						mAuthString = "WPA-EAP";
						mAuthMode = AuthModeWPA;
						break;
					}				
					
					mAuthString = "OPEN";
					mAuthMode = AuthModeOpen;
					
				}
			}
			
			Log.d(TAG, "Auth Mode = " + mAuthString);
			
			mAuthModeView.setText(mAuthString);
		}

		//init smartconnection
		IoTManager.InitSmartConnection();
//		Log.d(TAG, "Get password : " + Password +" by SSID = " + mConnectedSsid);
		mCheckBox.setOnCheckedChangeListener(mShowPaswordListener);
		mButtonStart.setOnClickListener(mButtonStartListener);
		mButtonStop.setOnClickListener(mButtonStopListener);
		mEditPassword.requestFocus();
		
	}
	
	CheckBox.OnCheckedChangeListener mShowPaswordListener = new CheckBox.OnCheckedChangeListener(){
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
		{
			if (isChecked)
			{
				mEditPassword.setInputType(0x90);
//				mEditPassword.setInputType(InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
//				mCheckBox.setText("Hide password");
			}
			else
			{
				mEditPassword.setInputType(0x81);
//				mEditPassword.setInputType(InputType.TYPE_CLASS_TEXT|InputType.TYPE_TEXT_VARIATION_PASSWORD);
//				mCheckBox.setText("Show password");
			}
		}
	};
	
	Button.OnClickListener mButtonStartListener = new Button.OnClickListener() {

		public void onClick(View arg0) {
			String SSID = mEditSSID.getText().toString();
			String Password = mEditPassword.getText().toString();
			String Target = mTargetEdit.getText().toString();

			Log.d(TAG, "Smart connection with : ssid = " + SSID +" Password = " 
				+ Password + " Target = " + Target);
			IoTManager.StartSmartConnection(SSID, Password, Target, (byte)mAuthMode);

			mButtonStart.setEnabled(false);
			mButtonStop.setEnabled(true);
//			mProgressBar.setVisibility(View.VISIBLE);
//			mProgressBar.incrementProgressBy(1);
//			mProgressBar.setProgress(120);
/*
			String Title="Configruation ...";
			String Message="Please waiting for smart connection complete!";
			
			mProgressDialog = new ProgressDialog(SmartConnection.this);
			mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
			mProgressDialog.setTitle(Title);
			mProgressDialog.setMessage(Message);
			mProgressDialog.setIcon(R.drawable.notepad);
			mProgressDialog.setIndeterminate(false);
			mProgressDialog.setProgress(30);
			mProgressDialog.setCancelable(true);

			mProgressDialog.show();
			new Thread()
			{
				int iTime = 20;
				int iCount = 0;
				public void run()
				{
					try
					{
						while (iCount < iTime)
						{
							mProgressDialog.setProgress(iCount++);
							Thread.sleep(1000);
						}
						mProgressDialog.cancel();
						IoTManager.StopSmartConnection();
					}
					catch (InterruptedException e)
					{
						mProgressDialog.cancel();
						IoTManager.StopSmartConnection();
					}
				}
			}.start();
			*/

//			}
		}
	};
	
	Button.OnClickListener mButtonStopListener = new Button.OnClickListener() {

		public void onClick(View arg0) {
			
			Log.d(TAG, "Smart connection Stop ");
			mButtonStart.setEnabled(true);
			mButtonStop.setEnabled(false);	
			IoTManager.StopSmartConnection();
		}
	};
}
