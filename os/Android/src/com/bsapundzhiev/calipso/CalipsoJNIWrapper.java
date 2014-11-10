/**
 * Calipso native wrapper
 */
package com.bsapundzhiev.calipso;

/**
 * @author Borislav Sapundzhiev 
 * 
 */
public class CalipsoJNIWrapper {

	static {
		System.loadLibrary("calipso-jni");
	}
	
	private native String stringFromJNI();
	private native void startCalipsoServer(final String filePath);
	/**
	 * Construct
	 */
	public CalipsoJNIWrapper() { }
	/**
	 * return server status
	 * @return {@link String}
	 */
	public String getStatus() {
		
		return stringFromJNI();
	}
	
	private Thread cpoThread;
	public void startCalipso(final String filePath){

		 cpoThread = new Thread(new Runnable() { 
		       public void run() {
		        	startCalipsoServer(filePath);
		        }    
		   });//.start();
		 
		 cpoThread.start();
	}
	
	public void stopCalipso() {
		
		cpoThread.interrupt();
	}
}
