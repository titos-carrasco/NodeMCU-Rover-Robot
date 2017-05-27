package rcr.robots.rover.wificar;

import android.util.Log;

import java.net.Socket;
import java.net.InetSocketAddress;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.BufferedReader;

public class RobotAPI {
    private Socket sock;
    private BufferedWriter out;
    private BufferedReader in;

    public RobotAPI() {}

    public boolean disconnect() {
        try {
            synchronized( this ) {
                if( sock != null )
                    sock.close();
                sock = null;
                Log.d( "RobotAPI/disconnect", "disconnected" );
                return true;
            }
        } catch ( Exception e ) {
            Log.d( "RobotAPI/disconnect", e.toString() );
            return false;
        }
    }

    public boolean connect( String ip, int port ) {
        disconnect();
        try {
            synchronized( this ) {
                sock = new Socket();
                sock.connect( new InetSocketAddress( ip, port ), 5000 );
                out = new BufferedWriter( new OutputStreamWriter( sock.getOutputStream() ) );
                in = new BufferedReader( new InputStreamReader( sock.getInputStream() ) );
                Log.d( "RobotAPI/connect", "connected" );
                return true;
            }
        } catch( Exception e ) {
            sock = null;
            Log.d( "RobotAPI/connect", e.toString() );
            return false;
        }
    }

    public boolean stop() { return moveRobot( 0, 0, 0 ); }

    public boolean forward( int speed ) { return moveRobot( speed, 1, 1 ); }

    public boolean backward( int speed ) {
        return moveRobot( speed, -1, -1 );
    }

    public boolean left( int speed ) {
        return moveRobot( speed, -1, 1 );
    }

    public boolean right( int speed ) {
        return moveRobot( speed, 1, -1 );
    }

    private boolean moveRobot( int speed, int left, int right ) {
        try {
            synchronized ( this ) {
                if( sock == null ) return false;
                String cmd = String.format( "%%MOTORS,%d,%d\n", speed*left, speed*right );
                out.write( cmd );
                out.flush();
                String resp = in.readLine();
                Log.d( "RobotAPI/moveRobot", cmd );
                Log.d( "RobotAPI/moveRobot", resp );
                return true;
            }
        } catch ( Exception e ) {
            Log.d( "RobotAPI/moveRobot", e.toString() );
            return false;
        }
    }

    public String getSensors() {
        try {
            synchronized( this ) {
                if( sock == null ) return null;
                String cmd = "%SENSORS\n";
                out.write( cmd );
                out.flush();
                String resp = in.readLine();
                Log.d( "RobotAPI/getSensors", cmd );
                Log.d( "RobotAPI/getSensors", resp );
                return resp;
            }
        } catch( Exception e ) {
            Log.d( "RobotAPI/getSensors", e.toString() );
            return null;
        }
    }

}