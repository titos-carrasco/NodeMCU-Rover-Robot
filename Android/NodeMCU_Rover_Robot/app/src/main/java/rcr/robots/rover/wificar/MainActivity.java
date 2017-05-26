package rcr.robots.rover.wificar;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.content.Intent;

import android.widget.EditText;

public class MainActivity extends AppCompatActivity {
    EditText entryDireccionIP;
    EditText entryPuerta;

    @Override
    protected void onCreate( Bundle savedInstanceState ) {
        super.onCreate( savedInstanceState );
        setContentView( R.layout.activity_main );

        entryDireccionIP = (EditText)findViewById( R.id.entryDireccionIP );
        entryPuerta = (EditText)findViewById( R.id.entryPuerta );

        entryDireccionIP.setText( R.string.DefaultIP );
        entryPuerta.setText( R.string.DefaultPort );
    }

    public void onClickConnect( View v ) {
        final String ip = entryDireccionIP.getText().toString();
        String strPort = entryPuerta.getText().toString();
        int port;
        try {
            port = Integer.parseInt( strPort );
        } catch ( Exception e ){
            port = -1;
        }

        Intent myIntent = new Intent( this, ControlActivity.class );
        myIntent.putExtra( "ServerIP", ip );
        myIntent.putExtra( "ServerPort", port );
        startActivity( myIntent );
    }

}
