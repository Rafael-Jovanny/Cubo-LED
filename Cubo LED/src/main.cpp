#include <Arduino.h>
// again


// ============================================================================
// CONFIGURACIÓN DE PINES (Mapeo de Hardware)
// ============================================================================
const int PIN_DS    = 4;  
const int PIN_SH_CP = 5;  
const int PIN_ST_CP = 6;  

// Pines hacia los transistores BD139 (Índice 0 = Piso de abajo, Pin 35)
const int PISOS[8] = {35, 36, 37, 38, 39, 40, 41, 42};

// ============================================================================
// VARIABLES Y MEMORIA DEL CUBO
// ============================================================================
// El "Buffer de Pantalla": Guarda el estado de cada led en coordenadas [Z][Y][X]
// 1 = Encendido, 0 = Apagado.
uint8_t cubo[8][8]; 

// Variables para el control de tiempo no bloqueante
unsigned long ultimoTiempoLluvia = 0;
const int VELOCIDAD_LLUVIA = 60; // Tiempo en ms para que la gota baje un piso

// ============================================================================
// FUNCIONES DE BAJO NIVEL (Multiplexación y Hardware)
// ============================================================================

/**
 * Envía un arreglo de 8 bytes (64 bits) a la cascada de 8 registros 74HC595
 */
void actualizarColumnas(uint8_t datos[8]) {
    digitalWrite(PIN_ST_CP, LOW);
    for (int i = 7; i >= 0; i--) {
        shiftOut(PIN_DS, PIN_SH_CP, MSBFIRST, datos[i]);
    }
    digitalWrite(PIN_ST_CP, HIGH);
}

/**
 * Limpia por completo la memoria del cubo (apaga todos los leds en el buffer)
 */
void limpiarCubo() {
    for (int z = 0; z < 8; z++) {
        for (int y = 0; y < 8; y++) {
            cubo[z][y] = 0;
        }
    }
}

/**
 * Dibuja un led en el buffer usando coordenadas espaciales.
 * @param x Columna (0 izquierda a 7 derecha)
 * @param y Fila (0 frente a 7 fondo)
 * @param z Piso (0 abajo a 7 arriba)
 */
void encenderLed(int x, int y, int z, bool estado) {
    if (x >= 0 && x < 8 && y >= 0 && y < 8 && z >= 0 && z < 8) {
        if (estado) {
            cubo[z][y] |= (1 << x);  // Enciende el bit correspondiente a la columna X
        } else {
            cubo[z][y] &= ~(1 << x); // Apaga el bit
        }
    }
}

/**
 * Función encargada de la multiplexación. Debe ejecutarse constantemente en el loop.
 * Enciende un piso a la vez a alta velocidad para engañar al ojo humano (Persistencia de la Visión).
 */
void refrescarCubo() {
    for (int z = 0; z < 8; z++) {
        // 1. Mandar los datos de las 64 columnas correspondientes a este piso Z
        actualizarColumnas(cubo[z]);
        
        // 2. Encender el transistor BD139 de este piso
        digitalWrite(PISOS[z], HIGH);
        
        // 3. Dejarlo encendido un instante micro-controlado para dar brillo
        delayMicroseconds(1500); 
        
        // 4. Apagar el piso para evitar "fantasmeo" antes de pasar al siguiente
        digitalWrite(PISOS[z], LOW);
    }
}

// ============================================================================
// LÓGICA DEL EFECTO DE LLUVIA
// ============================================================================
void calcularLluvia() {
    // 1. Desplazar todas las gotas existentes un piso hacia abajo (Física de caída)
    // Empezamos desde el piso 0 (abajo) hacia arriba para no sobreescribir los datos
    for (int z = 0; z < 7; z++) {
        for (int y = 0; y < 8; y++) {
            cubo[z][y] = cubo[z + 1][y];
        }
    }

    // 2. Limpiar el piso más alto (el cielo, Z=7) para recibir nuevas gotas
    for (int y = 0; y < 8; y++) {
        cubo[7][y] = 0;
    }

    // 3. Generar nuevas gotas en el "cielo" de forma aleatoria
    // Ajusta el número para controlar qué tan tormentosa es la lluvia
    int cantidadDeNuevasGotas = random(1, 4); 
    
    for (int i = 0; i < cantidadDeNuevasGotas; i++) {
        int xAleatorio = random(0, 8);
        int yAleatorio = random(0, 8);
        encenderLed(xAleatorio, yAleatorio, 7, true);
    }
}

// ============================================================================
// CONFIGURACIÓN INICIAL
// ============================================================================
void setup() {
    // Inicializar pines de datos
    pinMode(PIN_DS, OUTPUT);
    pinMode(PIN_SH_CP, OUTPUT);
    pinMode(PIN_ST_CP, OUTPUT);

    // Inicializar pines de pisos (BD139)
    for (int i = 0; i < 8; i++) {
        pinMode(PISOS[i], OUTPUT);
        digitalWrite(PISOS[i], LOW);
    }

    // Semilla para que el random no repita siempre el mismo patrón de gotas
    randomSeed(analogRead(0));

    limpiarCubo();
}

// ============================================================================
// BUCLE PRINCIPAL
// ============================================================================
void loop() {
    // Ejecutar constantemente la multiplexación para que el cubo nunca parpadee
    refrescarCubo();

    // Control de tiempo no bloqueante para la física de la lluvia
    unsigned long tiempoActual = millis();
    if (tiempoActual - ultimoTiempoLluvia >= VELOCIDAD_LLUVIA) {
        ultimoTiempoLluvia = tiempoActual;
        calcularLluvia(); // Calcula el siguiente cuadro de la animación
    }
}