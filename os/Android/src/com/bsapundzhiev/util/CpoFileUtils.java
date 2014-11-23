package com.bsapundzhiev.util;
/**
 * @author Borislav Sapundzhiev
 */
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;

import org.apache.http.conn.util.InetAddressUtils;

import android.R.array;
import android.content.Context;
import android.os.Environment;
import android.util.Log;


public class CpoFileUtils {
	public static final String CONFIG_FILE = "calipso.conf";
	public static final String MIME_TYPE_FILE = "mime.types";
	
	/**
	 * get application directory
	 * @param context
	 * @return {@link String}
	 * @throws Exception
	 */
	public static String getAppDataDir(Context context) throws Exception {
	    return context.getPackageManager()
	            .getPackageInfo(context.getPackageName(), 0)
	            .applicationInfo.dataDir;
	}
	
	public static File getCpoFilesDir(Context appCtx) {
		
		File filesDir;

		if (isExternalStorageWritable()) {
			// We can read and write the media
			filesDir = appCtx.getExternalFilesDir(null);
		} else {
			// Load another directory, probably local memory
			filesDir = appCtx.getFilesDir();
		}
		
		return filesDir;
	}
	
	/* Checks if external storage is available for read and write */
	public static boolean isExternalStorageWritable() {
	    String state = Environment.getExternalStorageState();
	    if (Environment.MEDIA_MOUNTED.equals(state)) {
	        return true;
	    }
	    return false;
	}
	
	public static String createExternalStoragePrivateFile(Context appCtx, InputStream is , String fileName) {
		
	    File file = new File( appCtx.getExternalFilesDir(null), fileName);

	    try {
	        //InputStream is = appCtx.getResources().openRawResource(R.raw.calipso);
	        OutputStream os = new FileOutputStream(file);
	        byte[] data = new byte[is.available()];
	        is.read(data);
	        os.write(data);
	        is.close();
	        os.close();
	    } catch (IOException e) {
	        // Unable to create file, likely because external storage is
	        // not currently mounted.
	        Log.w("ExternalStorage", "Error writing " + file, e);
	    }
	    
	    return file.getPath();
	}

	public static void deleteExternalStoragePrivateFile(Context appCtx, String fileName) {
	    // Get path for the file on external storage.  If external
	    // storage is not currently mounted this will fail.
	    File file = new File(appCtx.getExternalFilesDir(null), fileName);
	    if (file != null) {
	        file.delete();
	    }
	}

	public static boolean hasExternalStoragePrivateFile(Context appCtx, String fileName) {
	    
	    File file = new File(appCtx.getExternalFilesDir(null), fileName);
	    return file.exists();
	}
	//TODO: move to other place
	/**
	 * Get all device ip addresses
	 * @return {@link array}
	 */
	public static String[] getLocalIpAddress()
	{          
	    ArrayList<String> addresses = new ArrayList<String>();
	    try {
	        for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
	            NetworkInterface intf = en.nextElement();
	            for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
	                InetAddress inetAddress = enumIpAddr.nextElement();
	                if (!inetAddress.isLoopbackAddress()) {
	                    //IPAddresses.setText(inetAddress.getHostAddress().toString());
	                	boolean isIPv4 = InetAddressUtils.isIPv4Address(inetAddress.getHostAddress().toString());
	                	if(isIPv4) {
	                		addresses.add(inetAddress.getHostAddress().toString());
	                	}
	                }
	             }
	         }
	     } catch (SocketException ex) {
	         String LOG_TAG = null;
	         Log.e(LOG_TAG, ex.toString());
	     }
	    
	     return addresses.toArray(new String[0]);
	}
	
	public static long getUsedMemorySize() {

	    long freeSize = 0L;
	    long totalSize = 0L;
	    long usedSize = -1L;
	    try {
	        Runtime info = Runtime.getRuntime();
	        freeSize = info.freeMemory();
	        totalSize = info.totalMemory();
	        usedSize = totalSize - freeSize;
	    } catch (Exception e) {
	        e.printStackTrace();
	    }
	    return usedSize;
	}
}
