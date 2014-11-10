package com.bsapundzhiev.calipso;
import android.support.v4.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;


public class NetListFragment extends Fragment {

	ListView _netlistView ;
	CalipsoJNIWrapper _cpoWrapper;
	
    public NetListFragment() {
       
    	;
    }
    
    private void initServer() {
    	Context ctx = getActivity().getBaseContext();
    	
    	CpoFileUtils.createExternalStoragePrivateFile(ctx, 
				this.getResources().openRawResource(R.raw.mime), CpoFileUtils.cpoMime);
		
		String confPath = CpoFileUtils.createExternalStoragePrivateFile(ctx, 
				this.getResources().openRawResource(R.raw.calipso), CpoFileUtils.cpoFname);
		
		if(_cpoWrapper == null) {
			
			_cpoWrapper = new CalipsoJNIWrapper();
			_cpoWrapper.startCalipso(confPath);
		}
    }
    
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
    	
    	initServer();
    	
		View root = inflater.inflate(R.layout.activity_list_view_net, container, false);
        TextView textview = (TextView) root.findViewById(R.id.text1);
        textview.setText(_cpoWrapper.getStatus());
        
        // Get ListView object from xml
        _netlistView = (ListView) root.findViewById(R.id.list);
        // Defined Array values to show in ListView
        String[] values = CpoFileUtils.getLocalIpAddress();
 
        // Define a new Ad
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(getActivity().getBaseContext(),
                android.R.layout.simple_list_item_1, android.R.id.text1, values);
        
        // Assign adapter to ListView
        _netlistView.setAdapter(adapter); 
        
        _netlistView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, final View view,
                int position, long id) {
              final String item = (String) parent.getItemAtPosition(position);
           
              // Show Alert 
              Toast.makeText(getActivity().getBaseContext(), //,
                "Position :"+position+"  ListItem : " + item , Toast.LENGTH_LONG)
                .show();
            }
        });
        
        return root;
    }
}