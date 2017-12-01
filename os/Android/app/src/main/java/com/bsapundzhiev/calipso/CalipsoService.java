package com.bsapundzhiev.calipso;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.util.Log;
import android.widget.Toast;

import com.bsapundzhiev.util.CpoFileUtils;

public class CalipsoService extends Service {

    private static final String LOG_TAG = "CalipsoService";
    /**
     * Calipso JNI object
     */
    private static CalipsoJNIWrapper cpoHttpService = new CalipsoJNIWrapper();

    public CalipsoService() {}

    /**
     * Prepare configuration and initialize the server
     */
    private void initServer() throws Exception
    {

        Log.d(LOG_TAG, "App data folder: " + CpoFileUtils.getAppDataDir(this));
        Log.d(LOG_TAG, "App native lib folder: " + CpoFileUtils.getNativeLibraryDir(this));
        Log.d(LOG_TAG, CpoFileUtils.getCpoFilesDir(this.getBaseContext()).getPath() );

        if( cpoHttpService.isRunning() == false) {
            // Init config file
            String confPath = CpoFileUtils.getCpoFilesDir(this).toString() + "/" + CpoFileUtils.CONFIG_FILE;

            if(!CpoFileUtils.hasExternalStoragePrivateFile(this,CpoFileUtils.CONFIG_FILE)) {
                CpoFileUtils.createExternalStoragePrivateFile(this,
                        this.getResources().openRawResource(R.raw.mime), CpoFileUtils.MIME_TYPE_FILE);
            }

            if(!CpoFileUtils.hasExternalStoragePrivateFile(this, CpoFileUtils.CONFIG_FILE)) {
                confPath = CpoFileUtils.createExternalStoragePrivateFile(this,
                        this.getResources().openRawResource(R.raw.calipso), CpoFileUtils.CONFIG_FILE);
            }

            cpoHttpService.startCalipso(confPath);
        }
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        //throw new UnsupportedOperationException("Not yet implemented");
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        Log.d("Service", "Start...");
        try {
            initServer();
            Toast.makeText(this, "Calipso Service Started", Toast.LENGTH_LONG).show();
        } catch (Exception e) {

            Log.d(LOG_TAG, e.getMessage());
            Toast.makeText(this, "Error:" + e.getMessage(), Toast.LENGTH_LONG).show();
            return START_NOT_STICKY;
        }

        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d("Service", "Stop...");
        try {
            cpoHttpService.stopCalipso();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        Toast.makeText(this, "Service Destroyed", Toast.LENGTH_LONG).show();
    }
}
