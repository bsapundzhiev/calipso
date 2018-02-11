package com.bsapundzhiev.calipso;
/**
 * @author Borislav Sapundzhiev
 */
import java.util.Locale;

import android.Manifest;
import android.app.Activity;
import android.os.Build;
import android.os.Handler;
import android.os.ResultReceiver;
import android.support.v4.app.*;

import android.support.v4.content.ContextCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBar.*;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.content.pm.PackageManager;

public class MainActivity extends AppCompatActivity implements TabListener {

	/**
	 * The {@link android.support.v4.view.PagerAdapter} that will provide
	 * fragments for each of the sections. We use a {@link FragmentPagerAdapter}
	 * derivative, which will keep every loaded fragment in memory. If this
	 * becomes too memory intensive, it may be best to switch to a
	 * {@link android.support.v4.app.FragmentStatePagerAdapter}.
	 */
	private static final String TAG = "MainActivity";
	SectionsPagerAdapter mSectionsPagerAdapter;
    private static final int CPO_PERMISSIONS_REQUEST = 1;
	/**
	 * The {@link ViewPager} that will host the section contents
	 */
	ViewPager mViewPager;


    /**
     * request manifest permissions
     */
	private void checkAppPermission() {
		if (ContextCompat.checkSelfPermission(this,
				Manifest.permission.READ_CONTACTS)
				!= PackageManager.PERMISSION_GRANTED) {

			// Should we show an explanation?
			if (ActivityCompat.shouldShowRequestPermissionRationale(this,
					Manifest.permission.READ_CONTACTS)) {

				// Show an expanation to the user *asynchronously* -- don't block
				// this thread waiting for the user's response! After the user
				// sees the explanation, try again to request the permission.

			} else {

				// No explanation needed, we can request the permission.

				ActivityCompat.requestPermissions(this,
						new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
						CPO_PERMISSIONS_REQUEST);
			}
		}
	}
	@Override
	protected void onDestroy() {
		super.onDestroy();
		//stopService(this.getIntent());
		Log.d(TAG,"OnDestroy");
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	        
		super.onActivityResult(requestCode, resultCode, data);

	}

	public void startService(View view) {

        Intent serviceIntent = new Intent(getBaseContext(), CalipsoService.class);

        serviceIntent.putExtra(CalipsoService.BUNDLED_LISTENER, new ResultReceiver(new Handler()) {
            @Override
            protected void onReceiveResult(int resultCode, Bundle resultData) {
                super.onReceiveResult(resultCode, resultData);

                if (resultCode == Activity.RESULT_OK) {
                    String val = resultData.getString("value");
                    Log.i(TAG, "++++++++++++RESULT_OK+++++++++++ [" + val + "]");
                } else {
                    Log.i(TAG, "+++++++++++++RESULT_NOT_OK++++++++++++");
                }
            }
        });

		startService(serviceIntent);
	}

	public void stopService(View view) {
		stopService(new Intent(getBaseContext(), CalipsoService.class));
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		if (Build.VERSION.SDK_INT >= 23) {
			checkAppPermission();
		}


		
		// Set up the action bar.
        final ActionBar actionBar = this.getSupportActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
       
        // Create the adapter that will return a fragment for each of the three
     	// primary sections of the activity.
     	mSectionsPagerAdapter = new SectionsPagerAdapter(
     				getSupportFragmentManager());

     	// Set up the ViewPager with the sections adapter.
     	mViewPager = (ViewPager) findViewById(R.id.pager);
     	mViewPager.setAdapter(mSectionsPagerAdapter);

     	// When swiping between different sections, select the corresponding
     	// tab. We can also use ActionBar.Tab#select() to do this if we have
     	// a reference to the Tab.
     	mViewPager
     			.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
     				@Override
     				public void onPageSelected(int position) {
     						actionBar.setSelectedNavigationItem(position);
     					}
     				});

     		// For each of the sections in the app, add a tab to the action bar.
     		for (int i = 0; i < mSectionsPagerAdapter.getCount(); i++) {
     			// Create a tab with text corresponding to the page title defined by
     			// the adapter. Also specify this Activity object, which implements
     			// the TabListener interface, as the callback (listener) for when
     			// this tab is selected.
     			actionBar.addTab(actionBar.newTab()
     					.setText(mSectionsPagerAdapter.getPageTitle(i))
     					.setTabListener(this));
     		}
  
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {		
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		if (id == R.id.action_help) {
			
			Intent intent = new Intent(this, AboutHelpActivity.class);
	        this.startActivity(intent);
	        return true;
		}
		
		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onTabSelected(Tab tab, FragmentTransaction ft) {

	}

	@Override
	public void onTabReselected(Tab tab, FragmentTransaction fragmentTransaction) {
		// TODO Auto-generated method stub
	}
	
	@Override
	public void onTabUnselected(Tab tab, FragmentTransaction fragmentTransaction) {
		// TODO Auto-generated method stub
		
	}
	
	/**
	 * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
	 * one of the sections/tabs/pages.
	 */
	public class SectionsPagerAdapter extends FragmentPagerAdapter {

		public SectionsPagerAdapter(FragmentManager fm) {
			super(fm);
		}

		@Override
		public Fragment getItem(int position) {
			// getItem is called to instantiate the fragment for the given page.
			// Return a PlaceholderFragment (defined as a static inner class
			// below).
			switch (position) {
			case 0:
				return  new NetListFragment();
			default:
				return PlaceholderFragment.newInstance(position + 1);
			}
			
		}

		@Override
		public int getCount() {
			// Show 3 total pages.
			return 3;
		}

		@Override
		public CharSequence getPageTitle(int position) {
			Locale l = Locale.getDefault();
			switch (position) {
			case 0:
				return getString(R.string.title_section1).toUpperCase(l);
			case 1:
				return getString(R.string.title_section2).toUpperCase(l);
			case 2:
				return getString(R.string.title_section3).toUpperCase(l);
			}
			return null;
		}
	}
	
	/**
	 * A placeholder fragment containing a simple view.
	 */
	public static class PlaceholderFragment extends Fragment {
		/**
		 * The fragment argument representing the section number for this
		 * fragment.
		 */
		private static final String ARG_SECTION_NUMBER = "section_number";

		/**
		 * Returns a new instance of this fragment for the given section number.
		 */
		public static PlaceholderFragment newInstance(int sectionNumber) {
			
			PlaceholderFragment fragment = new PlaceholderFragment();
			Bundle args = new Bundle();
			args.putInt(ARG_SECTION_NUMBER, sectionNumber);
			fragment.setArguments(args);
			return fragment;
		}

		public PlaceholderFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			View rootView = inflater.inflate(R.layout.fragment_main, container,
					false);
			return rootView;
		}
	}
}
