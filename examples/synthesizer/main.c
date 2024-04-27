//==============================================================================
// File: ef-phasor-c/examples/synthesizer/main.c
// Author: Evelyn Fisher
// License: See LICENSE file
//==============================================================================

#include <stdio.h>

#define PHASOR_IMPLEMENTATION
#include "phasor.h"

#define FREQ 44100
#define DT (1.0/FREQ)

int main() {
    Phasor sines[3];                             // Create three phasors
    phasor_init(&sines[0], 261.63, DT, 0.0);     // Middle C
    phasor_init(&sines[1], 329.63, DT, 0.0);     // Middle E
    phasor_init(&sines[2], 392.00, DT, 0.0);     // Middle G

    // Open the output file. The audio is encoded as 44.1 kHz mono LE float64.
    FILE* out = fopen("output.raw", "w");
    if (out == NULL) {
        fprintf(stderr, "Failed to open file for writing.");
        return -1;
    }

    for (int i = 0; i < 4 * FREQ; i++) {         // Iterate for four seconds
        // Output the scaled sum of each phasor. Using y instead of x starts
        // each phasor at 0, avoiding a click at the beginning of the audio.
        double v = 0.1 * (sines[0].y + sines[1].y + sines[2].y);
        fwrite(&v, sizeof(double), 1, out);

        for (int j = 0; j < 3; j++) {
            phasor_clock(&sines[j]);             // Advance a timestep

            if (i % 1000 == 0) {
                phasor_correct(&sines[j]);           // Apply amplitude correction
            }
        }
    }

    fclose(out);                                 // Close the output file
}
