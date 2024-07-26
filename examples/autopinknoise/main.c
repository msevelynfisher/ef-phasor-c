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
// Gives the energy density of a pink noise distribution at the given frequency.
//------------------------------------------------------------------------------
double pink_edf(double f) {
    return 0.01 / f;
}

int main() {
    // Create a spectrum of phasors with energy proportional to 1/f
    Spectrum* spec = spectrum_generate(DT, 40.0, 4000.0, N, &pink_edf);

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
