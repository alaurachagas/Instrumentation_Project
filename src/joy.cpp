#include "Joystick.h"

#define JOY A1

const float V_MAX = 300.0;
const int JOY_CENTRO = 521;
const int ZONA_MORTA = 30;

void setupJoystick() {
  pinMode(JOY, INPUT);
}

float lerVelocidadeJoystick() {
  int leitura = analogRead(JOY);

  if (leitura > JOY_CENTRO - ZONA_MORTA && leitura < JOY_CENTRO + ZONA_MORTA) {
    return 0.0;
  }

  if (leitura < JOY_CENTRO) {
    return ((float)(JOY_CENTRO - leitura) / JOY_CENTRO) * V_MAX;
  } else {
    return -((float)(leitura - JOY_CENTRO) / (1023 - JOY_CENTRO)) * V_MAX;
  }
}