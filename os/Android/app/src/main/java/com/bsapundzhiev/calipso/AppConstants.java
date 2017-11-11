package com.bsapundzhiev.calipso;
/**
 * @author Borislav Sapundzhiev
 */
public class AppConstants {

	/**
	 * Default port to listen to 
	 */
	public static  String serverDefaultPort = "8080";  
	/**
	 * Calipso JNI instance
	 */
	private static CalipsoJNIWrapper cpoHttpService = new CalipsoJNIWrapper();
	
	public static CalipsoJNIWrapper getcpoHttpServiceHandle() 
	{
		return cpoHttpService;
	}
}
