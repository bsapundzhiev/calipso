/**
 * Calipso native wrapper
 */
package com.bsapundzhiev.calipso;

import android.util.Log;

import java.security.acl.LastOwnerException;

import static android.util.Log.*;

/**
 * @author Borislav Sapundzhiev  
 */
public class CalipsoJNIWrapper {

	private Thread _cpoThread;
	boolean isRunning;
	

	
	private native String getCurrentWorkingDir();
	private native String stringFromJNI();
	private native void startCalipsoServer(final String filePath);

	static {
		try {
			System.loadLibrary("calipso-jni");
		} catch (UnsatisfiedLinkError ex) {
			Log.d("CalipsoJNIWrapper", "calipso-jni failed to load: " + ex.getMessage());
		}
	}

	/**
	 * Construct
	 */
	public CalipsoJNIWrapper() { 
		isRunning = false;
	}
	
	/**
	 * return server status
	 * @return {@link String}
	 */
	public String getStatus() {
		
		return stringFromJNI();
	}
	
	public String getCurrentWorkingDirectory() {
		return getCurrentWorkingDir();
	}
 	
	public void startCalipso(final String filePath) {
		if(isRunning) return;
		
		_cpoThread = new Thread(new Runnable() { 
		       public void run() {
		       	isRunning = true;
		       	startCalipsoServer(filePath);

		       	Log.d("CalipsoJNIWrapper","Calipso jni procress ended.");

		       }  
		});

		_cpoThread.start();
	}
	
	public void stopCalipso() {
		
		_cpoThread.interrupt();
		isRunning = false;
	}
	
	public void restartCalipso() {
		//TODO:
	}
}
