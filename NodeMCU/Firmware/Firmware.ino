#include <Wire.h>
#include <ESP8266WiFi.h>

#define WIFI_SSID     "xxx"
#define WIFI_PASS     "xxx"
#define MyPort        1963

WiFiServer server( MyPort );

byte ledState = 0;
unsigned long lastBlink = 0;

#define CMD_LEN    64
char cmdRecv[ CMD_LEN ];

// Sensores y Actuadores
#include "SparkFunHTU21D.h"
HTU21D htu21d;
//ADC_MODE(ADC_VCC);

void setup()
{
  Serial.begin( 115200 );
  Serial.println();
  Serial.println();

  // necesario para especificar SDA y SCL ya que en D1 y D2 estan los motores
  Wire.begin( D5, D6 );

  // LED
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, HIGH );

  // Motores, PWM 1Khz, Duty 1023
  analogWriteFreq( 1000 );
  analogWriteRange( 1023 );

  // Power motores A (izquierda) y B (derecha)
  pinMode( D1, OUTPUT ); analogWrite( D1, 0 );
  pinMode( D2, OUTPUT ); analogWrite( D2, 0 );

  // Control motores A y B; HIGH=Forward, LOW=Backward
  pinMode( D3, OUTPUT ); digitalWrite( D3, HIGH );
  pinMode( D4, OUTPUT ); digitalWrite( D4, HIGH );

  // Sensores y Actuadores
  htu21d.begin();           // sensor de humedad/temperatura con I2C

  // La Wifi
  WiFi.setAutoConnect( true );
  WiFi.mode( WIFI_STA );
  WiFi.begin( WIFI_SSID, WIFI_PASS );

  Serial.print( "Conectando a la WiFi " );
  unsigned long start = millis();
  while( true )
  {
    Serial.print( "." );
    if( WiFi.status() == WL_CONNECTED )
    {
      Serial.print( " Conectado, mis datos son " );
      Serial.print( WiFi.localIP() );
      Serial.print( ':' );
      Serial.println( MyPort );
      break;
    }
    if( ( millis() - start ) > 15UL * 1000UL ) {
      Serial.println( " Timeout, cambiando a modo AP ..." );
      WiFi.mode( WIFI_AP );
      WiFi.softAPConfig( IPAddress( 192, 168, 1, 1 ), IPAddress( 192, 168, 1, 1 ), IPAddress( 255, 255, 255, 0 ) );
      WiFi.softAP( "RobotNodeMCU", "1234567890" );
      Serial.println( "Espere a que aparezca una red llamada RobotNodeMCU, clave 1234567890" );
      Serial.println( "Mis datos son 192.168.1.1:1963" );
      delay( 10000 );
      break;
    }
    delay( 500 );
  }

  // El server
  server.begin();
  Serial.println( "Esperando Conexiones..." );
}

void loop()
{
  // para indicar que estamos vivos
  heartBeat( 1000L );

  WiFiClient client = server.available();
  if( !client ) return;

  Serial.println();
  Serial.println( "Cliente Conectado" );
  Serial.println();
  while( client.connected() )
  {
    heartBeat( 500L );

    if( client.available() )
    {
      if( readFromClient( client, cmdRecv, CMD_LEN, 1000 ) )
      {
        Serial.println( cmdRecv );

        char *token = strtok( cmdRecv, "," );

        if( strcmp( token, "%MOTORS" ) == 0 )
        {
          token = strtok( 0, "," );
          int motorA = atoi( token );
          if( motorA < -1023 ) motorA = -1023;
          else if( motorA > 1023 ) motorA = 1023;
          token = strtok( 0, "," );
          int motorB = atoi( token );
          if( motorB < -1023 ) motorB = -1023;
          else if( motorB > 1023 ) motorB = 1023;

          analogWrite( D1, motorA > 0 ? motorA : -motorA );
          digitalWrite( D3, motorA > 0 ? HIGH : LOW );
          analogWrite( D2, motorB > 0 ? motorB : -motorB );
          digitalWrite( D4, motorB > 0 ? HIGH : LOW );

          client.println( "OK" );
        }
        else if( strcmp( token, "%SENSORS" ) == 0 )
        {
          char dataSend[32];

          //float vcc = ESP.getVcc()/1000.;
          //dtostrf( vcc, 1, 1, dataSend );
          unsigned int adc0 = analogRead( A0 );
          dtostrf( adc0, 1, 0, dataSend );
          yield();

          float temp = htu21d.readTemperature();
          strcat( dataSend, "," );
          dtostrf( temp, 1, 1, dataSend + strlen( dataSend ) );
          yield();

          float humd = htu21d.readHumidity();
          strcat( dataSend, "," );
          dtostrf( humd, 1, 1, dataSend + strlen( dataSend ) );
          yield();

          client.println( dataSend );
        }
      }
    }
  }

  // detenemos los motores
  analogWrite( D1, 0 );
  analogWrite( D2, 0 );

  // fin de la conexión
  Serial.println();
  Serial.println( "Cliente Desconectado" );
  Serial.println();
  Serial.println( "Esperando Conexiones..." );
}

int readFromClient( WiFiClient &client, char *buff, int len, long timeout )
{
  if( len <= 0 || timeout < 0 )
  {
    buff[0] = 0;
    return 0;
  }

  int i = 0;
  unsigned long t = millis();

  while( true )
  {
    yield();
    if( !client.connected() )
    {
      i = 0;
      break;
    }

    if( timeout && ( millis() - t ) >= timeout )
    {
      i = 0;
      break;
    }

    int c = client.read();
    if( c < 0 ) continue;

    if( c == '\n' ) break;

    buff[ i++ ] = c;

    if( i >= len )
    {
      i = 0;
      break;
    }
  }
  buff[i] = 0;
  return i;
}

void heartBeat( long d )
{
  unsigned long t = millis();
  if( t - lastBlink >= d ){
    ledState = ledState ? LOW : HIGH;
    digitalWrite( LED_BUILTIN, ledState );
    lastBlink = t;
  }
}

