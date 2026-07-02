#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

void setupMotorDC();
void controlMotorDC(int speed);
float PosicaoToSpeed(float posicao);

#endif