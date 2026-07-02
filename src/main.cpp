#include <Arduino.h>
#include <MotorPassos.h>
#include <JoyStick.h>

void setup() {
  Serial.begin(9600);

  setupJoystick();
  setupMotorPasso();
}

void loop() {
  float velocidade = lerVelocidadeJoystick();

  controlarMotorPasso(velocidade);

  static unsigned long tempoPrint = 0;

  if (millis() - tempoPrint >= 200) {
    tempoPrint = millis();

    Serial.print("Velocidade: ");
    Serial.println(velocidade);
  }
}