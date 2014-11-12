package com.bsapundzhiev.calipso;

public class AppConstants {

	
	private static CalipsoJNIWrapper cpoHttpService = new CalipsoJNIWrapper();
	
	public static CalipsoJNIWrapper getcpoHttpServiceHandle() 
	{
		return cpoHttpService;
	}
}
