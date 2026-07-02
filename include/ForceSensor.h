#ifndef FORCESENSOR_H
#define FORCESENSOR_H

#include <Arduino.h>

void setupForceSensor();
int calibrarZeroSensor();
float sensorParaPosicao(int leituraBruta);

#endif