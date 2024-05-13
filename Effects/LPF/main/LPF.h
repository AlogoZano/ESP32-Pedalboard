#ifndef LPF_H
#define LPF_H

typedef struct{

    float out;
    float filterCoef[2];
    float Fs;

} LowPassFilter;


void LPF_init(LowPassFilter* LPFstruct, float Fc, float Fs);

float LPF_update(LowPassFilter* LPFstruct, float input);

#endif