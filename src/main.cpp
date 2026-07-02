#include <Arduino.h>
#include <MotorPassos.h>
#include <JoyStick.h>
#include <ForceSensor.h>
#include <MotorDC.h>

float anguloReferencia = 0.0;

void setup() {
  Serial.begin(9600);
  setupJoystick();
  setupMotorPasso();
  setupForceSensor();
  setupMotorDC(anguloReferencia);
}

void loop() {
  int resistencia = analogRead(A0);
  int posicao = map(A0, 0, 900, 0, 100);
  anguloReferencia = float(map(resistencia, 0, 900, 0.0, 200.0));

  RunMotorDC(anguloReferencia);

  // float speed_dc = 
  // controlMotorDC(speed_dc);


  // float speed_step = lerVelocidadeJoystick();
  // controlarMotorPasso(speed_step);

  static unsigned long tempoPrint = 1000;

  if (millis() - tempoPrint >= 1000) {
    tempoPrint = millis();
    Serial.print("Resistencia: ");
    Serial.print(resistencia);
    Serial.print(" | Angulo de Referencia: ");
    Serial.println(anguloReferencia);
  }
}