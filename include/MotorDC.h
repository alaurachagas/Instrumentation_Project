#ifndef MOTOR_DC_H
#define MOTOR_DC_H

#include <Arduino.h>

// Inicialização
void setupMotorDC(float deslocamentoDesejado_graus);

// Deve ser chamada continuamente no loop()
void RunMotorDC(float deslocamentoDesejado_graus);

// Comandos

// Controle direto de PWM
// speed > 0: gira em um sentido
// speed < 0: gira no outro sentido
// speed = 0: parar

#endif