#include <Arduino.h>
#include <Wire.h>

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
}

void loop(){

  analogWrite( A_PWM, 50 ); digitalWrite( A_DIR, LOW );
  analogWrite( B_PWM, 50 ); digitalWrite( B_DIR, HIGH );
  delay( 2000 );

  analogWrite( A_PWM, 50 ); digitalWrite( A_DIR, LOW );
  analogWrite( B_PWM, 50 ); digitalWrite( B_DIR, LOW );
  delay( 2000 );
  
  analogWrite( A_PWM, 50 ); digitalWrite( A_DIR, HIGH );
  analogWrite( B_PWM, 50 ); digitalWrite( B_DIR, HIGH );
  delay( 2000 );
}
