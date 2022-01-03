#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>

WiFiClient net;
MQTTClient mqtt;

#define WIFI_SSID     "Tito's Network"
#define WIFI_PASS     "wifi password"

#define MQTT_PORT       1883
#define MQTT_SERVER   "test.mosquitto.org"
String  MQTT_CLIENTID = "Node_" + String( ESP.getChipId() );

#define MQTT_TOPIC    "rcr/robot/Robot-01"

#define SDA           9      // GPIO9                      * SDD2
#define SCL           10     // GPIO10                     * SDD3

#define A_PWM         D1     // GPIO5 - Motor A speed      * SCL
#define B_PWM         D2     // GPIO4 - Motor B speed      * SDA
#define A_DIR         D3     // GPIO0 - Motor A direction
#define B_DIR         D4     // GPIO2 - Motor B direction  * LED_BUILTIN

// en algunos shield invertir D3 y D4
//#define B_DIR         D3     // GPIO0 - Motor A direction
//#define A_DIR         D4     // GPIO2 - Motor B direction  * LED_BUILTIN

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

  // MQTT
  mqtt.begin( net );
  
  // nos conectamos
  while( !wifiReconnect() || !mqttReconnect() );
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


bool mqttReconnect (){
  if( mqtt.connected() )
    return true;

  // el servidor MQTT
  mqtt.setHost( MQTT_SERVER, MQTT_PORT );
  
  Serial.print( "Conectando a MQTT: ." );
  for( int i=0; i<10; i++ ){
    if( mqtt.connect( MQTT_CLIENTID.c_str() ) ){
      Serial.print( " --- " );
      Serial.print(  MQTT_SERVER );
      Serial.print(  ":" );
      Serial.print(  MQTT_PORT );
      Serial.println( " Ok" );

      mqtt.subscribe( MQTT_TOPIC, 0 );
      mqtt.onMessage( doReceiveMessage );
      
      return true;
    }

    Serial.print( "." );
    delay( 500 );
  }
  Serial.println( "Timeout" );
  return false;
}

void loop(){
  mqtt.loop();

  while( !wifiReconnect() || !mqttReconnect() ){
    mqtt.loop();
    yield();
  }
}

// procesamos el mensaje recibido
// {"lspeed":aaa,"rspeed":bbb}
void doReceiveMessage( String &topic, String &payload ) {
  Serial.println( payload );
  
  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson( json, payload);
  if( error ){
    Serial.println( "Error al deserializar JSON" );
    return;
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
