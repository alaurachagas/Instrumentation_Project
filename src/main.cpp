#include <Arduino.h>
#include <MotorPassos.h>
#include <JoyStick.h>
#include <ForceSensor.h>

#define R A0

void setup() {
  Serial.begin(9600);
  pinMode(R, INPUT);
  setupJoystick();
  setupMotorPasso();
  setupForceSensor();
}

void loop() {
  int resistencia = analogRead(R);
  int posicao = map(resistencia, 0, 900, 0, 100);
  
  
  // float speed_dc = 
  // controlMotorDC(speed_dc);

  
  // float speed_step = lerVelocidadeJoystick();
  // controlarMotorPasso(speed_step);

  static unsigned long tempoPrint = 1000;

  if (millis() - tempoPrint >= 200) {
    tempoPrint = millis();
    Serial.print("Resistencia: ");
    Serial.print(resistencia);
    Serial.print(" | Posicao: ");
    Serial.println(posicao);

  //   Serial.print("Velocidade: ");
  //   Serial.println(speed_step);
  }
}