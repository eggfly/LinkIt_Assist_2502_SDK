package mediatek.android.IoTManager;

import android.app.Activity;
import android.app.TabActivity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemClock;
import android.os.Handler;
import android.os.Message;

import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.TabHost.OnTabChangeListener;
import android.util.Log;

public class IoTManager extends TabActivity implements
OnTabChangeListener{    
	private TabHost myTabhost;
	
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}
    @Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	
		myTabhost = this.getTabHost();

		Intent iSmartConnection = new Intent(this,
				SmartConnection.class);		
		Intent iIoTManagement = new Intent(this,
				IoTManagement.class);		
		
		iSmartConnection.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		iIoTManagement.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		
		myTabhost.addTab(myTabhost.newTabSpec("SmartConnection").setIndicator("SmartConnection",
				null).setContent(iSmartConnection));
		myTabhost.addTab(myTabhost.newTabSpec("IoTManagement").setIndicator("Management",
				null).setContent(iIoTManagement));

		myTabhost.setOnTabChangedListener(this);	
	}

	public void onTabChanged(String arg0) {
		// TODO Auto-generated method stub
	} 
}
