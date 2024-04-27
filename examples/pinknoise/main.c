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
    Spectrum* spec = spectrum_new(N);
    for (int i = 0; i < N; i++) {
        double f = 40.0 + 5.0 * (i + randrange(0.0, 1.0));
        double phase = randrange(0.0, 2.0 * M_PI);
        double ampl = 0.1 * sqrt(1 / f);
        phasor_init(spectrum_phasor(spec, i), f, DT, phase);
        *spectrum_ampl(spec, i) = ampl;
    }

    // Open the output file. The audio is encoded as 44.1 kHz mono LE float64.
    FILE* out = fopen("output.raw", "w");
    if (out == NULL) {
        fprintf(stderr, "Failed to open file for writing.");
        return -1;
    }

    for (int i = 0; i < 4 * FREQ; i++) {         // Iterate for four seconds
        double v = spectrum_sum_x(spec);         // Sum the scaled phasors
        fwrite(&v, sizeof(double), 1, out);      // Output the sum

        spectrum_clock(spec);                    // Advance a timestep

        if (i % 1000 == 0) {
            spectrum_correct(spec);              // Apply amplitude correction
        }
    }

    spectrum_free(spec);                         // Deallocate spectrum
    fclose(out);                                 // Close the output file
}
