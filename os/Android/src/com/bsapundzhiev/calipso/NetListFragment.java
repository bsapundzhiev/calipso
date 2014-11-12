package com.bsapundzhiev.calipso;

import com.bsapundzhiev.util.CpoFileUtils;

import android.support.v4.app.Fragment;
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

	private ListView _netlistView ;
	
    public NetListFragment() {
    	
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
    	
    	
		View root = inflater.inflate(R.layout.activity_list_view_net, container, false);
        TextView textview = (TextView) root.findViewById(R.id.text1);
        textview.setText(AppConstants.getcpoHttpServiceHandle().getStatus());
        
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