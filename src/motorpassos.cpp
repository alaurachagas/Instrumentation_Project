#include "MotorPassos.h"

// Pinos das bobinas
#define B4 10
#define B3 5
#define B2 6
#define B1 9

const int bobinas[4] = {B1, B2, B3, B4};

const int PWM_DUTY = 200;

void setupMotorPasso() {
  for (int i = 0; i < 4; i++) {
    pinMode(bobinas[i], OUTPUT);
    analogWrite(bobinas[i], 0);
  }
}

void desligarBobinas() {
  for (int i = 0; i < 4; i++) {
    analogWrite(bobinas[i], 0);
  }
}

void acionarBobina(int indice) {
  for (int i = 0; i < 4; i++) {
    if (i == indice) {
      analogWrite(bobinas[i], PWM_DUTY);
    } else {
      analogWrite(bobinas[i], 0);
    }
  }
}

void controlarMotorPasso(float v) {
  static int etapaAtual = -1;
  static unsigned long tempoAnterior = 0;

  if (v > -0.5 && v < 0.5) {
    desligarBobinas();
    etapaAtual = -1;
    return;
  }

  float velocidade = abs(v);

  unsigned long intervaloPasso_us = 1000000.0 / velocidade;
  unsigned long agora = micros();

  if (agora - tempoAnterior >= intervaloPasso_us) {
    tempoAnterior = agora;

    if (etapaAtual == -1) {
      if (v > 0) {
        etapaAtual = 0;
      } else {
        etapaAtual = 3;
      }
    } else {
      if (v > 0) {
        etapaAtual++;
        if (etapaAtual > 3) etapaAtual = 0;
      } else {
        etapaAtual--;
        if (etapaAtual < 0) etapaAtual = 3;
      }
    }

    acionarBobina(etapaAtual);
  }
}