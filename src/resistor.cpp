#include "ForceSensor.h"

#define FORCE_SENSOR A0

int offsetSensor = 0;

const int SENSOR_MIN = 0;
const int SENSOR_MAX = 900;

int lerSensorEstavel() {
  analogRead(FORCE_SENSOR);      // descarta primeira leitura
  delayMicroseconds(100);
  return analogRead(FORCE_SENSOR);
}

int calibrarZeroSensor() {
  long soma = 0;
  const int N = 50;

  for (int i = 0; i < N; i++) {
    soma += lerSensorEstavel();
    delay(5);
  }

  return soma / N;
}

float sensorParaPosicao(int leituraBruta) {
  int leituraCorrigida = leituraBruta - offsetSensor;

  leituraCorrigida = constrain(leituraCorrigida, 0, SENSOR_MAX - offsetSensor);

  float posicao = leituraCorrigida * 100.0 / (SENSOR_MAX - offsetSensor);
  if (posicao > 100){
    posicao = 100.0;
  } else if (posicao < 0){
    posicao = 0.0;
  }

  return posicao;
}

void setupForceSensor() {
    pinMode(FORCE_SENSOR, INPUT);
    offsetSensor = calibrarZeroSensor();
}