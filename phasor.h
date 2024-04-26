//==============================================================================
// File: ef-phasor-c/phasor.h
//==============================================================================
// Copyright 2024 Evelyn Fisher
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//==============================================================================

#ifndef PHASOR_H_INCLUDED
#define PHASOR_H_INCLUDED

#include <stdlib.h>
#include <math.h>

#ifdef PHASOR_USE_SINGLE_PRECISION
    typedef float ph_t;
#else
    typedef double ph_t;
#endif

typedef struct {
    ph_t x, y, c, s;
} Phasor;

typedef struct {
    int len;
    ph_t* ampls;
    Phasor* phasors;
} Spectrum;

//------------------------------------------------------------------------------
// Initializes a phasor with frequency f, timestep dt, and initial phase, th, in
// radians.
//------------------------------------------------------------------------------
void phasor_init(Phasor* phasor, ph_t f, ph_t dt, ph_t th);

//------------------------------------------------------------------------------
// Alters the frequency and timestep of a phasor. Note that modifying either of
// these values requires evaluating trigonometric functions, so using this
// function at every timestep negates the computational advantages of using this
// library.
//------------------------------------------------------------------------------
void phasor_update(Phasor* phasor, ph_t f, ph_t dt);

//------------------------------------------------------------------------------
// Clock the phasor, advancing its x and y values to the next timestep.
//------------------------------------------------------------------------------
void phasor_clock(Phasor* phasor);

//------------------------------------------------------------------------------
// Over many cycles, the amplitude of a phasor may drift. Calling this function
// makes a small correction to the amplitude without evaluating a square root by
// performing a single Newton-Raphson iteration of the function
//     f(c) = 1 - (c*r)^2
//------------------------------------------------------------------------------
void phasor_correct(Phasor* phasor);

//------------------------------------------------------------------------------
// DEPRECATED - Loops over an array of phasors, calling phasor_clock on each.
//------------------------------------------------------------------------------
void phasors_clock(Phasor* phasors, int n);

//------------------------------------------------------------------------------
// DEPRECATED - Loops over an array of phasors, calling phasor_correct on each.
//------------------------------------------------------------------------------
void phasors_correct(Phasor* phasors, int n);

//------------------------------------------------------------------------------
// Allocate a new Spectrum of phasors.
//------------------------------------------------------------------------------
Spectrum* spectrum_new(int n);

//------------------------------------------------------------------------------
// Free the memory associated with a Spectrum.
//------------------------------------------------------------------------------
void spectrum_free(Spectrum* s);

//------------------------------------------------------------------------------
// Get a pointer to the amplitude of the ith Phasor in a Spectrum.
//------------------------------------------------------------------------------
ph_t* spectrum_ampl(Spectrum* s, int i);

//------------------------------------------------------------------------------
// Get a pointer to the ith Phasor in a Spectrum.
//------------------------------------------------------------------------------
Phasor* spectrum_phasor(Spectrum* s, int i);

//------------------------------------------------------------------------------
// Clock each Phasor in a Spectrum, advancing the timestep.
//------------------------------------------------------------------------------
void spectrum_clock(Spectrum* s);

//------------------------------------------------------------------------------
// Apply a correction term to each Phasor in Spectrum to avoid amplitude drift.
//------------------------------------------------------------------------------
void spectrum_correct(Spectrum* s);

//------------------------------------------------------------------------------
// Compute the sum of the Phasor x-values (scaled by their amplitudes).
//------------------------------------------------------------------------------
ph_t spectrum_sum_x(Spectrum* s);

//------------------------------------------------------------------------------
// Compute the sum of the Phasor y-values (scaled by their amplitudes).
//------------------------------------------------------------------------------
ph_t spectrum_sum_y(Spectrum* s);

//==============================================================================
// Implementation
//==============================================================================
// This section needs to be incorporated into exactly one translation unit by
// defining the following macro.
//==============================================================================

#ifdef PHASOR_IMPLEMENTATION

void phasor_init(Phasor* phasor, ph_t f, ph_t dt, ph_t th) {
    phasor->x = cos(th);
    phasor->y = sin(th);
    ph_t dtheta = 2.0 * M_PI * f * dt;
    phasor->c = cos(dtheta);
    phasor->s = sin(dtheta);
}

void phasor_update(Phasor* phasor, ph_t f, ph_t dt) {
    ph_t dtheta = 2.0 * M_PI * f * dt;
    phasor->c = cos(dtheta);
    phasor->s = sin(dtheta);
}

void phasor_clock(Phasor* phasor) {
    ph_t nx = phasor->c * phasor->x - phasor->s * phasor->y;
    ph_t ny = phasor->s * phasor->x + phasor->c * phasor->y;
    phasor->x = nx;
    phasor->y = ny;
}

void phasor_correct(Phasor* phasor) {
    // Using the function f(c) = 1-(c*r)^2, perform a single iteration of
    // Newton-Raphson root finding to make a small correction to the
    // amplitude of the phasor. This correction is not exact but is very
    // computational efficient as it avoids computing a square root.
    ph_t r2 = phasor->x * phasor->x + phasor->y * phasor->y;
    ph_t c = (1 + r2) / (2 * r2);
    phasor->x *= c;
    phasor->y *= c;
}

// DEPRECATED
void phasors_clock(Phasor* phasors, int n) {
    for (int i = 0; i < n; ++i) {
        phasor_clock(&phasors[i]);
    }
}

// DEPRECATED
void phasors_correct(Phasor* phasors, int n) {
    for (int i = 0; i < n; ++i) {
        phasor_correct(&phasors[i]);
    }
}

Spectrum* spectrum_new(int n) {
    void* mem = malloc(sizeof(Spectrum) + n * (sizeof(ph_t) + sizeof(Phasor)));
    if (mem == NULL) {
        return NULL;
    }

    Spectrum* s = mem;
    s->len = n;
    s->ampls = mem + sizeof(Spectrum);
    s->phasors = mem + sizeof(Spectrum) + n * sizeof(ph_t);

    return s;
}

void spectrum_free(Spectrum* s) {
    free(s);
}

ph_t* spectrum_ampl(Spectrum* s, int i) {
    if (i >= s->len) {
        return NULL;
    }
    
    return &s->ampls[i];
}

Phasor* spectrum_phasor(Spectrum* s, int i) {
    if (i >= s->len) {
        return NULL;
    }

    return &s->phasors[i];
}

void spectrum_clock(Spectrum* s) {
    for (int i = 0; i < s->len; i++) {
        phasor_clock(&s->phasors[i]);
    }
}

void spectrum_correct(Spectrum* s) {
    for (int i = 0; i < s->len; i++) {
        phasor_correct(&s->phasors[i]);
    }
}

ph_t spectrum_sum_x(Spectrum* s) {
    ph_t sum = 0.0;

    for (int i = 0; i < s->len; i++) {
        sum += s->ampls[i] * s->phasors[i].x;
    }

    return sum;
}

ph_t spectrum_sum_y(Spectrum* s) {
    ph_t sum = 0.0;

    for (int i = 0; i < s->len; i++) {
        sum += s->ampls[i] * s->phasors[i].y;
    }

    return sum;
}

#endif // PHASOR_IMPLEMENTATION

#endif // PHASOR_H_INCLUDED
