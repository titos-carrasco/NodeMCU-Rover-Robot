package rcr.robots.rover.wificar;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.DragEvent;
import android.view.View;
import android.util.Log;

import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.SeekBar;
import android.widget.Toast;

public class ControlActivity extends AppCompatActivity {
    private RobotAPI robot;
    private volatile boolean thStop = false;
    private int lastMove = R.id.botonDetener;

    @Override
    protected void onCreate( Bundle savedInstanceState ) {
        super.onCreate( savedInstanceState );
        setContentView( R.layout.activity_control );

        showConnecting();
        final String ip = getIntent().getStringExtra( "ServerIP" );
        final int port = getIntent().getIntExtra( "ServerPort", -1 );
        robot = new RobotAPI();

        SeekBar sb = (SeekBar)findViewById( R.id.botonVelocidad );
        sb.setOnSeekBarChangeListener( new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged( SeekBar seekBar, int progress, boolean user)  {
                onNavButtonClicked( findViewById( lastMove ) );
            }

            @Override
            public void onStartTrackingTouch( SeekBar seekBar ) {}

            @Override
            public void onStopTrackingTouch( SeekBar seekBar ) {}
        });

        new Thread( new Runnable() {
            @Override
            public void run() {
                if( robot.connect( ip, port ) ) {
                    hideConnecting();
                    new Thread( new Runnable() {
                        @Override
                        public void run() {
                            try {
                                int n = 0;
                                while( !thStop ) {
                                    if( n == 0 ) {
                                        String sensors = robot.getSensors();
                                        showSensors(sensors);
                                        n = 50;
                                    }
                                    Thread.sleep( 100 );
                                    n--;
                                }
                                Log.d( "ControlActivity/Sensors", "finished" );
                            } catch ( Exception e ) {
                                Log.d( "ControlActivity/Sensors", e.toString() );
                            }
                        }
                    }).start();
                } else {
                    showToast( getString( R.string.CannotConnect ) );
                    goBack();
                }
            }
        }).start();
    }

    @Override
    protected void onStop() {
        super.onStop();
        thStop = true;
        new Thread( new Runnable() {
            @Override
            public void run() {
                robot.stop();
                robot.disconnect();
            }
        }).start();
        Log.d( "ControlActivity/onStop", "stopping" );
    }

    public void onNavButtonClicked( final View v ) {
        new Thread( new Runnable() {
            @Override
            public void run() {
                boolean ret = false;
                int speed = ((SeekBar)findViewById( R.id.botonVelocidad )).getProgress();
                switch ( v.getId() ) {
                    case R.id.botonDetener:
                        ret = robot.stop();
                        break;
                    case R.id.botonAvanzar:
                        ret = robot.forward( speed );
                        break;
                    case R.id.botonRetroceder:
                        ret = robot.backward( speed );
                        break;
                    case R.id.botonIzquierda:
                        ret = robot.left( speed );
                        break;
                    case R.id.botonDerecha:
                        ret = robot.right( speed );
                        break;
                }
                if( !ret ) {
                    showToast( getString( R.string.ConnectionLost ) );
                }
                else
                    lastMove = v.getId();
            }
        }).start();
    }

    private void showConnecting() {
        runOnUiThread( new Runnable() {
            @Override
            public void run() {
                findViewById( R.id.botonAvanzar ).setVisibility( View.GONE );
                findViewById( R.id.botonRetroceder ).setVisibility( View.GONE );
                findViewById( R.id.botonIzquierda ).setVisibility( View.GONE );
                findViewById( R.id.botonDerecha ).setVisibility( View.GONE );
                findViewById( R.id.botonDetener ).setVisibility( View.GONE );
                findViewById( R.id.botonVelocidad ).setVisibility( View.GONE );
                findViewById( R.id.textTemperatura ).setVisibility( View.GONE );
                findViewById( R.id.textHumedad ).setVisibility( View.GONE );
                findViewById( R.id.textVoltaje ).setVisibility( View.GONE );
                findViewById( R.id.connecting).setVisibility( View.VISIBLE );
            }
        } );
    }

    private void hideConnecting() {
        runOnUiThread( new Runnable() {
            @Override
            public void run() {
                findViewById( R.id.botonAvanzar ).setVisibility( View.VISIBLE );
                findViewById( R.id.botonRetroceder ).setVisibility( View.VISIBLE );
                findViewById( R.id.botonIzquierda ).setVisibility( View.VISIBLE );
                findViewById( R.id.botonDerecha ).setVisibility( View.VISIBLE );
                findViewById( R.id.botonDetener ).setVisibility( View.VISIBLE );
                findViewById( R.id.botonVelocidad ).setVisibility( View.VISIBLE );
                findViewById( R.id.textTemperatura ).setVisibility( View.VISIBLE );
                findViewById( R.id.textHumedad ).setVisibility( View.VISIBLE );
                findViewById( R.id.textVoltaje ).setVisibility( View.VISIBLE );
                findViewById( R.id.connecting).setVisibility( View.GONE );
            }
        } );
    }

    private void showSensors( final String sensors ) {
        runOnUiThread( new Runnable() {
            @Override
            public void run() {
                String data[] = sensors.split( "," );
                ((TextView)findViewById( R.id.textVoltaje )).setText( data[0] );
                ((TextView)findViewById( R.id.textTemperatura )).setText( data[1] + "°" );
                ((TextView)findViewById( R.id.textHumedad )).setText( data[2] + "%" );
            }
        } );
    }

    private void showToast( final String msg ) {
        runOnUiThread( new Runnable() {
            @Override
            public void run() {
                Toast.makeText(getApplicationContext(), msg, Toast.LENGTH_SHORT).show();
            }
        } );
    }

    private void goBack() {
        runOnUiThread( new Runnable() {
            @Override
            public void run() {
                finish();
            }
        } );
    }
}
