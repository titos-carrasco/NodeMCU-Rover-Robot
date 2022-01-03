#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

WiFiClient net;

#define WIFI_SSID     "Tito's Network"
#define WIFI_PASS     "xxxx"

#define SDA           9      // GPIO9                      * SDD2
#define SCL           10     // GPIO10                     * SDD3

#define A_PWM         D1     // GPIO5 - Motor A speed      * SCL
#define B_PWM         D2     // GPIO4 - Motor B speed      * SDA
#define A_DIR         D3     // GPIO0 - Motor A direction
#define B_DIR         D4     // GPIO2 - Motor B direction  * LED_BUILTIN

// en algunos shield invertir D3 y D4
//#define B_DIR         D3     // GPIO0 - Motor A direction
//#define A_DIR         D4     // GPIO2 - Motor B direction  * LED_BUILTIN

// este server
#define MYPORT        1963
WiFiServer server( MYPORT );

#define CMD_LEN    128
char cmdRecv[ CMD_LEN ];

void setup(){
  Serial.begin( 9600 );
  while ( !Serial ) delay( 50 );
  Serial.println();
  Serial.println();

  // movemos SDA y SCL tal que no interfieran con los motores
  Wire.begin( SDA, SCL );

  // Rango 0-100
  analogWriteRange( 100 );

  // Apaga motores A (izquierda) y B (derecha)
  pinMode( A_PWM, OUTPUT ); analogWrite( A_PWM, 0 );
  pinMode( B_PWM, OUTPUT ); analogWrite( B_PWM, 0 );

  // Control motores A y B; LOW=Forward, HIGH=Backward
  pinMode( A_DIR, OUTPUT ); digitalWrite( A_DIR, LOW );
  pinMode( B_DIR, OUTPUT ); digitalWrite( B_DIR, LOW );


  // la WIFI
  WiFi.mode( WIFI_STA );
  WiFi.setAutoConnect( true );
  WiFi.begin( WIFI_SSID, WIFI_PASS );

  // nos conectamos
  while( !wifiReconnect() );

  // El server
  server.begin();
  Serial.print( "Esperando Conexiones en " );
  Serial.print( WiFi.localIP() );
  Serial.print( ":" );
  Serial.println( MYPORT );
}

bool wifiReconnect(){
  if( WiFi.status() == WL_CONNECTED )
    return true;

  Serial.print( "Conectando a la WiFi: ." );
  for( int i=0; i<10; i++ ){
    if( WiFi.status() == WL_CONNECTED ){
      Serial.print( " --- " );
      Serial.print( WiFi.localIP() );
      Serial.println( " Ok" );
      return true;
    }

    Serial.print( "." );
    delay( 500 );
  }
  Serial.println( "Timeout" );
  return false;
}

void loop(){
  // la wifi debe estar conectada
  if( !wifiReconnect() ) return;

  // esperamos por una conexion
  WiFiClient client = server.available();
  if( !client ) return;

  // tenemos una conexion
  Serial.println();
  Serial.println( "Cliente Conectado" );
  Serial.println();
  while( client.connected() ){
    yield();
  
    // esperamos el comando
    if( !readFromClient( client, cmdRecv, CMD_LEN, 1000 ) )
      continue;

    // procesamos el mensaje recibido
    // {"lspeed":aaa,"rspeed":bbb}
    Serial.println( cmdRecv );
    
    StaticJsonDocument<512> json;
    DeserializationError error = deserializeJson( json, cmdRecv);
    if( error ){
      Serial.println( "Error al deserializar JSON" );
      continue;
    }
  
    int lSpeed = json["lspeed"];
    int rSpeed = json["rspeed"];
    Serial.print( lSpeed );
    Serial.print( " " );
    Serial.println( rSpeed );
  
    int speed = lSpeed >= 0 ? lSpeed : -lSpeed;
    int dir = lSpeed >= 0 ? LOW : HIGH;
    Serial.print( speed );
    Serial.print( " " );
    Serial.println( dir );  
    analogWrite( A_PWM, speed );
    digitalWrite( A_DIR, dir);
  
    
    speed = rSpeed >= 0 ? rSpeed : -rSpeed;
    dir = rSpeed >= 0 ? LOW : HIGH;
    Serial.print( speed );
    Serial.print( " " );
    Serial.println( dir );  
    analogWrite( B_PWM, speed );
    digitalWrite( B_DIR, dir );
  }

  // detenemos los motores
  analogWrite( A_PWM, 0 ); digitalWrite( A_DIR, LOW );
  analogWrite( B_PWM, 0 ); digitalWrite( B_DIR, LOW );

  // fin de la conexión
  Serial.println();
  Serial.println( "Cliente Desconectado" );
  Serial.println();

  Serial.print( "Esperando Conexiones en " );
  Serial.print( WiFi.localIP() );
  Serial.print( ":" );
  Serial.println( MYPORT );
}


//---
int readFromClient( WiFiClient &client, char *buff, unsigned int len, unsigned long timeout ){
  int i = 0;
  unsigned long t0 = millis();
  while( true ) {
    if( !client.connected() ){
      i = 0;
      break;
    }

    int c = client.read();
    if( c < 0 ){
      if( timeout && ( millis() - t0 ) >= timeout ){
        i = 0;
        break;
      }
      continue;
    }
    t0 = millis();

    if( c == '\n' ) break;
    buff[ i++ ] = c;
    if( i >= len )
      break;
  }
  
  buff[i] = 0;
  return i;
}
