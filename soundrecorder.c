#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "portaudio.h"

#define SAMPLE_RATE 58000
#define DURATION 4.5
#define NUM_CHANNELS 1
#define FRAMES_PER_BUFFER 512

PaStream *stream;
float recordedSamples[DURATION * SAMPLE_RATE]; // Array to store recorded samples

// Callback function to record audio samples
static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData) {
    const float *input = (const float *)inputBuffer;
    float *output = (float *)outputBuffer;
    unsigned long i;
    (void) outputBuffer; /* Prevent unused variable warning. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    for (i = 0; i < framesPerBuffer; i++) {
        recordedSamples[i] = input[i]; // Store recorded samples
    }

    return paContinue;
}

int main(void) {
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    // Open a stream to record audio
    err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, 0, paFloat32, SAMPLE_RATE,
                               FRAMES_PER_BUFFER, paCallback, NULL);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        goto done;
    }

    // Start the stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        goto done;
    }

    printf("Recording...\n");

    // Record for DURATION seconds
    Pa_Sleep(DURATION * 1000);

    // Stop the stream
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        goto done;
    }

    // Plot recorded samples using gnuplot
    FILE *gnuplotPipe = popen("gnuplot -persist", "w");
    if (gnuplotPipe) {
        fprintf(gnuplotPipe, "plot '-' with lines\n");
        for (int i = 0; i < DURATION * SAMPLE_RATE; i++) {
            fprintf(gnuplotPipe, "%f\n", recordedSamples[i]);
        }
        fprintf(gnuplotPipe, "e\n");
        fflush(gnuplotPipe);
        fclose(gnuplotPipe);
    } else {
        fprintf(stderr, "Failed to open gnuplot pipe.\n");
    }

done:
    // Cleanup
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
