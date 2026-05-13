#ifndef TYVERI_SIKRING_CONTROL_H
#define TYVERI_SIKRING_CONTROL_H

#include <Arduino.h>

/**
 * Main update function for burglary security state machine.
 * Call once per loop iteration.
 */
void tyveri_sikring_control_update();

#endif // TYVERI_SIKRING_CONTROL_H
