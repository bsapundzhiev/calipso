/**
 * Calipso server JNI wrapper
 * @author Borislav Sapundzhiev
 *
 */
package com.bsapundzhiev.calipso;

import android.util.Log;
import java.util.concurrent.atomic.AtomicBoolean;

public class CalipsoJNIWrapper {

	private Thread _cpoThread;

    private AtomicBoolean isRunning = new AtomicBoolean(false);

	private native String getCurrentWorkingDir();
	private native String stringFromJNI();
	private native void startCalipsoServer(final String filePath);
    private native void stopCalipsoServer();

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
	public CalipsoJNIWrapper() {}
	
	/**
	 * return server status
	 * @return {@link String}
	 */
	public String getStatus() { return stringFromJNI(); }
    /**
     * Get current working directoru
     * @return {@link String}
     */
	public String getCurrentWorkingDirectory() {
		return getCurrentWorkingDir();
	}
 	
	public void startCalipso(final String filePath) {


		if(isRunning.get() == true) return;

		_cpoThread = new Thread(new Runnable() {

		    public void run() {

		       	isRunning.set(true);
		       	startCalipsoServer(filePath);
		    }
		});

		_cpoThread.start();
	}

    /**
     * Stop server
     * @throws InterruptedException
     */
	public void stopCalipso() throws InterruptedException {

	    stopCalipsoServer();
        //_cpoThread.interrupt();
        _cpoThread.join();
		isRunning.set(false);
        Log.d("CalipsoJNIWrapper","stopCalipso");
	}

	public boolean isRunning()
    {
       return isRunning.get();
    }
}
