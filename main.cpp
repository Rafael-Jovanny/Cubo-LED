#include <Arduino.h>

// Tu fila de pines consecutiva y segura
const uint8_t PINES_CAPAS[8] = {1, 2, 42, 40, 39, 38, 37, 36};

// Pines para los registros 74HC595 (Columnas)
const uint8_t PIN_DATA  = 11;
const uint8_t PIN_LATCH = 12;
const uint8_t PIN_CLOCK = 13;

// Configuración de PWM para la versión v2.x del Core
const uint32_t FRECUENCIA_PWM = 5000; // 5 kHz
const uint8_t  RESOLUCION_PWM = 8;    // 8 bits (0-255)

void setup() {
    // 1. Inicializar y encender todas las columnas permanentemente
    pinMode(PIN_DATA, OUTPUT);
    pinMode(PIN_LATCH, OUTPUT);
    pinMode(PIN_CLOCK, OUTPUT);

    digitalWrite(PIN_LATCH, LOW);
    for (uint8_t i = 0; i < 8; i++) {
        shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, 0xFF); 
    }
    digitalWrite(PIN_LATCH, HIGH);

    // 2. Configuración por pasos obligatoria en v2.x
    for (uint8_t i = 0; i < 8; i++) {
        // Primero: Configurar canal, frecuencia y resolución (Canales 0 al 7)
        ledcSetup(i, FRECUENCIA_PWM, RESOLUCION_PWM);
        
        // Segundo: Asociar el pin físico al canal de hardware
        ledcAttachPin(PINES_CAPAS[i], i);
        
        // Tercero: Inicializar el brillo del canal en 0
        ledcWrite(i, 0); 
    }
}

void loop() {
    // Barrido ascendente con efecto fade (respiración)
    for (uint8_t c = 0; c < 8; c++) {
        
        // FADE IN: Incrementa el brillo apuntando al CANAL 'c'
        for (int16_t brillo = 0; brillo <= 255; brillo++) {
            ledcWrite(c, brillo); 
            delay(3); 
        }
        
        delay(150); // Mantener encendida a full

        // FADE OUT: Reduce el brillo apuntando al CANAL 'c'
        for (int16_t brillo = 255; brillo >= 0; brillo--) {
            ledcWrite(c, brillo);
            delay(3);
        }
        
        delay(50); // Breve pausa antes de pasar a la siguiente capa
    }
}