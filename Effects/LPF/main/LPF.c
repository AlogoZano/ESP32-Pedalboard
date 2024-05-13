#include "LPF.h"
#include <math.h>
#define PI 3.14159265358979323846

void LPF_init(LowPassFilter* LPFstruct, float Fc, float Fs){
    /*Inicialización de filtro (Frecuencia de muestreo y valor inicial de salida)*/
    LPFstruct->Fs = Fs;
    LPFstruct->out = 0.0f;

    /*Inicialización coeficientes del filtro*/
    float alpha = (2*PI)*(Fc/LPFstruct->Fs);  /*alpha = (2*pi)*(Fc/Fs) */

    LPFstruct->filterCoef[0] = alpha/(1.0f + alpha); 
    LPFstruct->filterCoef[1] = 1.0f/(1.0f + alpha);
}

float LPF_update(LowPassFilter* LPFstruct, float input){

/*Dada la función de transferencia:

Vout[n] = (alpha/(1+alpha))*(Vin[n]) + ((1/(1+alpha))*Vout[n-1])

*/

LPFstruct->out = (LPFstruct->filterCoef[0]*input)+(LPFstruct->filterCoef[1]*LPFstruct->out);

return LPFstruct->out;

}

