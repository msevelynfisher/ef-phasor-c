//==============================================================================
// File: ef-phasor-c/examples/pinknoise/main.c
// Author: Evelyn Fisher
// License: See LICENSE file
//==============================================================================

#include <stdio.h>
#include <stdlib.h>

#define PHASOR_IMPLEMENTATION
#include "phasor.h"

#define FREQ 44100
#define DT (1.0/FREQ)
#define N 1000

//------------------------------------------------------------------------------
// Generates a random double between min and max.
//------------------------------------------------------------------------------
double randrange(double min, double max) {
    double unit = (double)rand() / (double)RAND_MAX;
    return min + unit * (max - min);
}

int main() {
    // Create a spectrum of phasors with energy proportional to 1/f
    double ampls[N];
    Phasor sines[N];
    for (int i = 0; i < N; i++) {
        double f = 40.0 + 5.0 * (i + randrange(0.0, 1.0));
        ampls[i] = 0.1 * sqrt(1 / f);
        phasor_init(&sines[i], f, DT, randrange(0.0, 2.0 * M_PI));
    }

    // Open the output file. The audio is encoded as 44.1 kHz mono LE float64.
    FILE* out = fopen("output.raw", "w");
    if (out == NULL) {
        fprintf(stderr, "Failed to open file for writing.");
        return -1;
    }

    for (int i = 0; i < 4 * FREQ; i++) {         // Iterate for four seconds
        double v = 0.0;                          // Sum the scaled phasors
        for (int j = 0; j < N; j++) {
            v += ampls[j] * sines[j].y;
        }
        fwrite(&v, sizeof(double), 1, out);      // Output the sum

        phasors_clock(sines, N);                 // Advance a timestep

        if (i % 1000 == 0) {
            phasors_correct(sines, N);           // Apply amplitude correction
        }
    }

    fclose(out);                                 // Close the output file
}
