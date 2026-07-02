#include "MotorDC.h"
#include <math.h>

// ==========================================================
// PINOS
// ==========================================================

// Mantive os pinos do código original.
// Se você trocar para outros pinos, altere somente aqui.

#define ENB 11
#define INA 12
#define INB 13

#define ENCODER_A 2
#define ENCODER_B 3

// -------------------- ENCODER --------------------
volatile long pulsos = 0;
volatile byte estadoAnterior = 0;

// Pulsos por volta do eixo de saída
const float N = 460.0;

// Use -1 se PWM positivo faz a posição medida diminuir.
// Use +1 se PWM positivo faz a posição medida aumentar.
const int SENTIDO_ENCODER = -1;

// -------------------- AMOSTRAGEM --------------------
const unsigned long Ts_ms = 20;
const float Ts = Ts_ms / 1000.0;


float deslocamentoDesejadoAnterior_graus = 0.0;
const float variacaoMinimaReferencia_graus = 0.5;
bool primeiraReferencia = true;

float velReferencia_RPM = 20.0;


// PARÂMETROS DE SEGURANÇA DA POSIÇÃO
float antecipacaoFreio_graus = 85.0;
float toleranciaPos_graus = 8.0;

// Freio ativo
const bool usarFreioAtivo = true;
const unsigned long tempoFreio_ms = 250;


// PI de velocidade
float Kp_vel = 0.15;
float Ki_vel = 0.08;

float integralVel = 0.0;
float integralMin = -70.0;
float integralMax = 70.0;

// Filtro de velocidade
float alpha = 0.10;
float rpmFiltrado = 0.0;
const int PWM_MAX = 255;

// Você mediu que PWM 50 não gira e PWM 80 gira.
const int PWM_MIN_MOVIMENTO = 80;

// Rate limiter do PWM
float pwmAnterior = 0.0;
float taxaSubidaPWM = 350.0;     // PWM/s
float taxaDescidaPWM = 2500.0;   // PWM/s


long pulsosZero = 0;
long pulsosAnterior = 0;

float posicao_graus = 0.0;
float posAlvo_graus = 0.0;
float posFreio_graus = 0.0;

int sentidoMovimento = 0;

unsigned long tempoAnterior = 0;
unsigned long instanteFreio = 0;

enum EstadoControle {
  MOVENDO,
  FREANDO,
  CONCLUIDO
};

EstadoControle estado = MOVENDO;

void atualizarEncoder() {
  byte estadoAtual = 0;

  if (digitalRead(ENCODER_A)) estadoAtual |= 0b01;
  if (digitalRead(ENCODER_B)) estadoAtual |= 0b10;

  byte transicao = (estadoAnterior << 2) | estadoAtual;

  if (
    transicao == 0b0001 ||
    transicao == 0b0111 ||
    transicao == 0b1110 ||
    transicao == 0b1000
  ) {
    pulsos++;
  }
  else if (
    transicao == 0b0010 ||
    transicao == 0b1011 ||
    transicao == 0b1101 ||
    transicao == 0b0100
  ) {
    pulsos--;
  }

  estadoAnterior = estadoAtual;
}

float limitar(float valor, float minimo, float maximo) {
  if (valor > maximo) return maximo;
  if (valor < minimo) return minimo;
  return valor;
}

int sinal(float valor) {
  if (valor > 0) return 1;
  if (valor < 0) return -1;
  return 0;
}

float aplicarRateLimiter(
  float valorDesejado,
  float valorAnterior,
  float taxaSubida,
  float taxaDescida
) {
  float delta = valorDesejado - valorAnterior;

  float deltaMaxSubida = taxaSubida * Ts;
  float deltaMaxDescida = taxaDescida * Ts;

  if (delta > deltaMaxSubida) {
    return valorAnterior + deltaMaxSubida;
  }

  if (delta < -deltaMaxDescida) {
    return valorAnterior - deltaMaxDescida;
  }

  return valorDesejado;
}

void aplicarPWM(float pwm) {
  pwm = limitar(pwm, -PWM_MAX, PWM_MAX);

  int pwmAbs = abs((int)pwm);

  if (pwm > 0) {
    digitalWrite(INA, HIGH);
    digitalWrite(INB, LOW);
    analogWrite(ENB, pwmAbs);
  }
  else if (pwm < 0) {
    digitalWrite(INA, LOW);
    digitalWrite(INB, HIGH);
    analogWrite(ENB, pwmAbs);
  }
  else {
    digitalWrite(INA, LOW);
    digitalWrite(INB, LOW);
    analogWrite(ENB, 0);
  }
}

void frearMotor() {
  if (usarFreioAtivo) {
    // Freio dinâmico na ponte H
    digitalWrite(INA, HIGH);
    digitalWrite(INB, HIGH);
    analogWrite(ENB, 255);
  }
  else {
    aplicarPWM(0);
  }
}

void pararDefinitivo() {
  aplicarPWM(0);

  integralVel = 0.0;
  pwmAnterior = 0.0;

  estado = CONCLUIDO;
}

// ==========================================================
// FUNÇÕES PÚBLICAS
// ==========================================================

void setupMotorDC(float deslocamentoDesejado_graus) {
  pinMode(ENB, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  estadoAnterior = 0;
  if (digitalRead(ENCODER_A)) estadoAnterior |= 0b01;
  if (digitalRead(ENCODER_B)) estadoAnterior |= 0b10;

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), atualizarEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), atualizarEncoder, CHANGE);

  noInterrupts();
  pulsosZero = pulsos;
  pulsosAnterior = pulsos;
  interrupts();

  posicao_graus = 0.0;

  posAlvo_graus = deslocamentoDesejado_graus;
  sentidoMovimento = sinal(deslocamentoDesejado_graus);

  deslocamentoDesejadoAnterior_graus = deslocamentoDesejado_graus;
  primeiraReferencia = false;

  if (sentidoMovimento == 0) {
    estado = CONCLUIDO;
  }
  else {
    estado = MOVENDO;
  }

  float margemEfetiva = antecipacaoFreio_graus;
  float limiteMargem = fabs(posAlvo_graus) * 0.90;

  if (margemEfetiva > limiteMargem) {
    margemEfetiva = limiteMargem;
  }

  posFreio_graus = posAlvo_graus - sentidoMovimento * margemEfetiva;

  tempoAnterior = millis();

  aplicarPWM(0);
}

void RunMotorDC(float deslocamentoDesejado_graus) {
  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoAnterior >= Ts_ms) {
    tempoAnterior += Ts_ms;

    // -------------------- ENCODER --------------------
    noInterrupts();
    long pulsosAtual = pulsos;
    interrupts();

    long deltaPulsos = pulsosAtual - pulsosAnterior;
    pulsosAnterior = pulsosAtual;

    // -------------------- POSIÇÃO RELATIVA --------------------
    posicao_graus = SENTIDO_ENCODER * ((pulsosAtual - pulsosZero) * 360.0) / N;

    // -------------------- VELOCIDADE --------------------
    float rpmMedido = SENTIDO_ENCODER * (deltaPulsos * 60.0) / (N * Ts);
    rpmFiltrado = alpha * rpmMedido + (1.0 - alpha) * rpmFiltrado;

    // -------------------- ATUALIZAÇÃO DA REFERÊNCIA --------------------
    // Esta parte verifica se o ângulo de referência mudou.
    // Se mudou, recalcula o alvo, o sentido de movimento e a posição de freio.
    if (
      primeiraReferencia ||
      fabs(deslocamentoDesejado_graus - deslocamentoDesejadoAnterior_graus) > variacaoMinimaReferencia_graus
    ) {
      deslocamentoDesejadoAnterior_graus = deslocamentoDesejado_graus;
      primeiraReferencia = false;

      posAlvo_graus = deslocamentoDesejado_graus;

      float erroReferencia = posAlvo_graus - posicao_graus;
      sentidoMovimento = sinal(erroReferencia);

      integralVel = 0.0;
      pwmAnterior = 0.0;

      if (sentidoMovimento == 0 || fabs(erroReferencia) <= toleranciaPos_graus) {
        estado = CONCLUIDO;
        aplicarPWM(0);
      }
      else {
        estado = MOVENDO;

        float margemEfetiva = antecipacaoFreio_graus;
        float limiteMargem = fabs(erroReferencia) * 0.90;

        if (margemEfetiva > limiteMargem) {
          margemEfetiva = limiteMargem;
        }

        posFreio_graus = posAlvo_graus - sentidoMovimento * margemEfetiva;
      }
    }

    float erroAlvo = posAlvo_graus - posicao_graus;
    float erroAbs = fabs(erroAlvo);

    float progresso = sentidoMovimento * posicao_graus;
    float progressoFreio = sentidoMovimento * posFreio_graus;
    float progressoAlvo = sentidoMovimento * posAlvo_graus;

    float velRef_RPM = 0.0;
    float pwmCmd = 0.0;

    if (estado == MOVENDO) {

      bool chegouNaRegiaoFreio = progresso >= progressoFreio;
      bool chegouNoAlvo = progresso >= progressoAlvo;
      bool chegouPorTolerancia = erroAbs <= toleranciaPos_graus;

      // Segurança
      if (chegouNaRegiaoFreio || chegouNoAlvo || chegouPorTolerancia) {
        estado = FREANDO;
        instanteFreio = millis();

        integralVel = 0.0;
        pwmAnterior = 0.0;
        pwmCmd = 0.0;

        frearMotor();
      }
      else {

        velRef_RPM = sentidoMovimento * velReferencia_RPM;

        // PI DE VELOCIDADE
        float erroVel = velRef_RPM - rpmFiltrado;

        integralVel += Ki_vel * Ts * erroVel;
        integralVel = limitar(integralVel, integralMin, integralMax);

        float pwmDesejado = Kp_vel * erroVel + integralVel;
        pwmDesejado = limitar(pwmDesejado, -PWM_MAX, PWM_MAX);

        // Trava de sinal:
        // Não deixa o controlador inverter o sentido.
        if (pwmDesejado * sentidoMovimento < 0) {
          pwmDesejado = 0.0;
          integralVel = 0.0;
        }

        if (fabs(pwmDesejado) < PWM_MIN_MOVIMENTO) {
          pwmDesejado = sentidoMovimento * PWM_MIN_MOVIMENTO;
        }

        pwmCmd = aplicarRateLimiter(
          pwmDesejado,
          pwmAnterior,
          taxaSubidaPWM,
          taxaDescidaPWM
        );

        pwmAnterior = pwmCmd;

        aplicarPWM(pwmCmd);
      }
    }

    else if (estado == FREANDO) {
      frearMotor();

      if (millis() - instanteFreio >= tempoFreio_ms) {
        pararDefinitivo();
      }
    }

    else if (estado == CONCLUIDO) {
      aplicarPWM(0);

      velRef_RPM = 0.0;
      pwmCmd = 0.0;
      integralVel = 0.0;
      pwmAnterior = 0.0;
    }
  }
}