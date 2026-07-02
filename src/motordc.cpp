#include "MotorDC.h"

// Pinos Ponte H
#define INA 13
#define INB 12
#define ENB 11

// Pinos Encoder
#define ENCODER_A 7
#define ENCODER_B 8

void setupMotorDC() {
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
}

void controlMotorDC(int speed) {
    // Controla a velocidade do motor DC usando PWM e direção
    
    // Calculo velocidade para PWM

    if (speed > 0) {
        digitalWrite(INA, HIGH);
        digitalWrite(INB, LOW);
        analogWrite(ENB, speed);
    } else if (speed < 0) {
        digitalWrite(INA, LOW);
        digitalWrite(INB, HIGH);
        analogWrite(ENB, -speed);
    } else {
        // Parar o motor
        digitalWrite(INA, LOW);
        digitalWrite(INB, LOW);
        analogWrite(ENB, 0);
    }
}

float PosicaoToSpeed(float posicao) {
  // Converte a posição em velocidade

}
