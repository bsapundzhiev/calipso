/**
 * Calipso native wrapper
 */
package com.bsapundzhiev.calipso;

/**
 * @author Borislav Sapundzhiev  
 */
public class CalipsoJNIWrapper {

	private Thread _cpoThread;
	boolean isRunning;
	
	static {
		System.loadLibrary("calipso-jni");
	}
	
	private native String getCurrentWorkingDir();
	private native String stringFromJNI();
	private native void startCalipsoServer(final String filePath);
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
