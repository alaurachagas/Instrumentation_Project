#include <Arduino.h>

// Pino do joystick
#define JOY A1

// Pinos das bobinas
#define B4 3
#define B3 5
#define B2 6
#define B1 9

const int bobinas[4] = {B1, B2, B3, B4};

// Duty cycle aplicado na bobina ativa
// Arduino padrão: 0 a 255
const int PWM_DUTY = 200;

// Velocidade máxima em passos por segundo
const float V_MAX = 500.0;

// Centro real do joystick
const int JOY_CENTRO = 521;

// Zona morta ao redor do centro
// Evita que o motor fique tremendo quando o joystick está solto
const int ZONA_MORTA = 1;

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

// Converte a leitura do joystick em velocidade
// 0    ->  100.0
// 521  ->    0.0
// 1023 -> -100.0
float joystickParaVelocidade(int leitura) {
  if (leitura > JOY_CENTRO - ZONA_MORTA && leitura < JOY_CENTRO + ZONA_MORTA) {
    return 0.0;
  }

  if (leitura < JOY_CENTRO) {
    // Região de 0 até 521
    // Quanto menor a leitura, maior a velocidade positiva
    return ((float)(JOY_CENTRO - leitura) / JOY_CENTRO) * V_MAX;
  } else {
    // Região de 521 até 1023
    // Quanto maior a leitura, maior a velocidade negativa
    return -((float)(leitura - JOY_CENTRO) / (1023 - JOY_CENTRO)) * V_MAX;
  }
}

// v em passos por segundo
// v > 0: horário
// v < 0: anti-horário
// v = 0: parado
void controlarMotorPasso(float v) {
  static int etapaAtual = -1;
  static unsigned long tempoAnterior = 0;

  if (v == 0.0) {
    desligarBobinas();
    etapaAtual = -1;
    return;
  }

  float velocidade;

  if (v > 0) {
    velocidade = v;
  } else {
    velocidade = -v;
  }

  unsigned long intervaloPasso_us = 1000000.0 / velocidade;
  unsigned long agora = micros();

  if (agora - tempoAnterior >= intervaloPasso_us) {
    tempoAnterior = agora;

    if (etapaAtual == -1) {
      if (v > 0) {
        etapaAtual = 0;  // B1
      } else {
        etapaAtual = 3;  // B4
      }
    } else {
      if (v > 0) {
        etapaAtual++;
        if (etapaAtual > 3) {
          etapaAtual = 0;
        }
      } else {
        etapaAtual--;
        if (etapaAtual < 0) {
          etapaAtual = 3;
        }
      }
    }

    acionarBobina(etapaAtual);
  }
}

void setup() {
  pinMode(JOY, INPUT);

  setupMotorPasso();
}

void loop() {
  int leituraJoystick = analogRead(JOY);

  float velocidade = joystickParaVelocidade(leituraJoystick);

  controlarMotorPasso(velocidade);
}