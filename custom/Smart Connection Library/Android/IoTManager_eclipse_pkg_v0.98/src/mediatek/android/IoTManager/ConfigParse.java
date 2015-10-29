package mediatek.android.IoTManager;

import java.io.*;
import java.util.*;

import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

public class ConfigParse {
	
	private List<NetworkBlock> sectList = new ArrayList<NetworkBlock>();
	private String ConfPath = null;
	private String NetworkBlockStart = "network={";
	private String NetworkBlockEnd = "}";
	String TAG = "ConfigParse";
	
	public ConfigParse(String FileName) 
	{
		try
		{		
			Log.d(TAG, "Configture file = " + FileName);
			int iRet = readFromFile(FileName);
			if (-1 == iRet)
			{
				//return -1;
			}
		}
		catch (FileNotFoundException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
			Log.d(TAG, "file does not exist" + FileName);
			//throws IOException;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		//return 0;
	}
	
	private int readFromFile(String ConfFile) throws IOException
	{
		File SourceFile = new File(ConfFile);
		FileInputStream Fin = null;
		NetworkBlock mNetworkBlock = null;
		Log.d("IoT config: ", "readFromFile : " + ConfFile);
		try {
			if (!SourceFile.exists())
			{
//				SourceFile.createNewFile();
//				initConfFile(ConfFile);
				Log.d("IoT config: ", "SourceFile does not exist");
//				return -1; 
			}

			Fin = new FileInputStream(SourceFile);

		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
	Log.d("IoT config: ", "Open readFromFile : " + ConfFile);
		InputStream in = new BufferedInputStream(Fin);
		BufferedReader br = new BufferedReader(new InputStreamReader(in));
		
		String Line;
		
		while ((Line = br.readLine()) != null)
		{
			//处理空行
			if (Line.equals(""))
			{
				continue;
			}
			
			if (Line.equals(NetworkBlockStart))
			{		
				mNetworkBlock = new NetworkBlock();
				Log.d("IoT config: ", "New block================");
				sectList.add(mNetworkBlock);
				continue;
			}

			if (Line.equals(NetworkBlockEnd))
			{		
				break;
			}
			
			//if (null != SectionArr[iIndex])
			if (null != mNetworkBlock)
			{
				mNetworkBlock.setNetworkBlockKeyValue(Line);
				Log.d("IoT config: ", "Value = " + Line);
			}
		}
		
		System.out.println(sectList.size());
		return 0;
	}
	
	private NetworkBlock getNetworkBlock(String KeyWord, String Value)
	{
		NetworkBlock result = null;
		for (int i = 0; i < sectList.size(); i ++)
		{
			result = sectList.get(i);
			if (result.getValueByKey(KeyWord).equals(Value))
			{
				return result;
			}
		}
		
		return result;
	}
	
	/*
		we have two block as follows:
		block1{
			KeyWord1 = testItem1
			KeyWord2 = password1
			KeyWord3 = hello1
		}
		block1{
			KeyWord1 = testItem2
			KeyWord2 = password2
			KeyWord3 = hello2
		}
		
		this founction will found out the value of Keyword2 in the block witch Keyword1 match KeyValue. 
	*/
	public String getSpecValueByKey(String KeyWord1, String KeyValue, String KeyWord2)
	{
		String result = null;
		NetworkBlock tagNetworkBlock = getNetworkBlock(KeyWord1, KeyValue);
		Log.d("IoT config: ", " KeyWord1 = " + KeyWord1 +" KeyValue = " + KeyValue + " KeyWord2 = " + KeyWord2);
		if (null != tagNetworkBlock)
		{
			result = tagNetworkBlock.getValueByKey(KeyWord2);
		}
		return result;
	}

	class NetworkBlock
	{
		private List<String> keyValueList = new ArrayList<String>();
		public NetworkBlock()
		{
		}

		public String getValueByKey(String Key)
		{
			String Value = "";
			String tmpString = "";
			int iStart = 0;
			int iLength = 0;
			
			for (int i = 0; i < keyValueList.size(); i ++)
			{
				tmpString = keyValueList.get(i);
				if (tmpString.startsWith(Key))
				{
					iStart = tmpString.indexOf("=");
					iLength = tmpString.length();
					Value = tmpString.substring(iStart + 1, iLength);
					Value = Value.trim();
					
					if (Value.startsWith("\"") && Value.endsWith("\""))
					{
						Value = Value.replace("\"", "");
						Value = Value.trim();
					}
					
				}
			}
			
			return Value;
		}

		public List<String> getNetworkBlockKeyValue()
		{
			return this.keyValueList;
		}
		
		public void setNetworkBlockKeyValue(String keyValue)
		{
			this.keyValueList.add(keyValue);
		}
	
	}
}
