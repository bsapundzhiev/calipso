package com.bsapundzhiev.calipso;
/**
 * @author Borislav Sapundzhiev
 */
import com.bsapundzhiev.util.CpoFileUtils;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

public class AboutHelpActivity extends ActionBarActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_about_help);
		
		TextView memView = (TextView) findViewById(R.id.mem_info);
		memView.setText(String.format("Memory used: %d Kb", CpoFileUtils.getUsedMemorySize() / 1024L));
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		//getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		
		// Respond to the action bar's Up/Home button
		if(id == 0x102002c || id == R.id.home) {
			Log.d("ID", Integer.toHexString(id));
			finish();
			return true;
		}
	
		if (id == R.id.action_settings) {
			return true;
		}
		
		return super.onOptionsItemSelected(item);
	}

}
