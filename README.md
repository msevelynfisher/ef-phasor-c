ef-phasor-c
===

Many applications that perform sine and cosine computations do so in fixed time intervals. Direct computation of sine and cosine are relatively expensive. This single header library provides a simple interface for efficiently computing sines and cosines by taking advantage of information from previous timesteps. It does so by applying fixed rotations to phasors.

Usage
===

This is a single header library. Simply include it in your C file. At least one translation unit should have

    #define PHASOR_IMPLEMENTATION
    #include "phasor.h"

The easiest way to get started is by looking at the examples directory.

Accuracy
===

This library is designed especially for physical modeling applications where phase exactness is not only not required, but gradual phase drift is more realistic and therefore acceptable.
