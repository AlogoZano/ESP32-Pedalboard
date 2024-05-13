#include "Tremolo.h"

void Tremolo_Init(Tremolo* tremStruct, float depth, float lfoFreq, float Fs){

    Tremolo_SetDepth(tremStruct, depth);

    tremStruct->Fs = Fs;
    tremStruct->lfoCount = 0.0f;
    tremStruct->lfoDir = 1.0f;
    tremStruct->out = 0.0f;

    Tremolo_LFOFreq(tremStruct, lfoFreq);
}

void Tremolo_SetDepth(Tremolo* tremStruct, float depth){

    if(depth < 0.0f){
        depth = 0.0f;
    }else if(depth > 1.0f){
        depth = 1.0f;
    }

    tremStruct->depth = depth;
}

void Tremolo_LFOFreq(Tremolo* tremStruct, float lfoFreq){

    if(lfoFreq <= 0.0f){
        lfoFreq = 1.0f;
    }else if(lfoFreq > (0.5f * tremStruct->Fs)){
        lfoFreq = 0.5f * tremStruct->Fs;
    }

    tremStruct->lfoCountLimit = 0.25f * (tremStruct->Fs/lfoFreq);

    if(tremStruct->lfoCount > tremStruct->lfoCountLimit){
        tremStruct->lfoCount = tremStruct->lfoCountLimit;

    }else if(tremStruct->lfoCount < -tremStruct->lfoCountLimit){
        tremStruct->lfoCount = -tremStruct->lfoCountLimit;
    }

}

float Tremolo_Update(Tremolo* tremStruct, float input){
    //yn = ((X_n*depth)*(G_n)) + ((X_n)*(1-depth))
    tremStruct->out = ((input*tremStruct->depth)*(tremStruct->lfoCount))+((input)*(1.0f-tremStruct->depth));

    if(tremStruct->lfoCount >= tremStruct->lfoCountLimit){
        tremStruct->lfoDir = -1.0f;

    }else if(tremStruct->lfoCount <= -tremStruct->lfoCountLimit){
        tremStruct->lfoDir = +1.0f;
    }

    tremStruct->lfoCount += tremStruct->lfoDir;

    return (tremStruct->out);
}