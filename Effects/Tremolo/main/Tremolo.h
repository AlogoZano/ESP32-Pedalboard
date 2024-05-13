#ifndef Tremolo_H
#define Tremolo_H


typedef struct{

    //Depth de tremolo
    float depth;

    //Generación de LFO (Low frequency oscillator)
    float lfoDir;
    float lfoCount;
    float lfoCountLimit;

    //Cualidades de efecto
    float Fs;
    float out;

} Tremolo;


void Tremolo_Init(Tremolo* tremStruct, float depth, float lfoFreq, float Fs);

void Tremolo_SetDepth(Tremolo* tremStruct, float depth);
void Tremolo_LFOFreq(Tremolo* tremStruct, float lfoFreq);

float Tremolo_Update(Tremolo* tremStruct, float input);

#endif