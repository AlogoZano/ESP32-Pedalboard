/* FILTRO PASA BAJAS DE PRIMER ORDEN
Código de muestreo de ADC y escritura en DAC. 8-01-23
El siguiente código lee el ADC cada determinado tiempo
gracias al uso de asignación de timers por interrupción y adc.h
posteriormente se procesa el valor y se escribe al DAC para volver a ser 
interpretado como señal analógica.

--NOTA--
Es importante mencionar que este código usa la librería OBSOLETA
de ADC y DAC. se recomienda cambiar por esp/continuous o esp/oneshot

Es importante conocer la teoría de los filtros digitales

--ADRIÁN LOZANO GONZÁLEZ--IRS.

*/

#include <stdio.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include "sdkconfig.h"
#include "LPF.c"

#define CHANNEL ADC1_CHANNEL_4
#define SAMPLES 64

#define MicroSeconds 0.000001
#define TsMicro 100

//ADC
uint16_t xN = 0;
unsigned int val_Smooth = 0;

//TIMERS
uint64_t period = TsMicro;

//DAC
uint8_t dac_value = 0.0f;
float yN = 0.0f;

//FX
LowPassFilter LPFilter;
float FreCort = 500;
float FreSamp = 1.0f/(TsMicro*(MicroSeconds));

//Config
esp_err_t set_ADC(void);
esp_err_t set_Timer(void);
esp_err_t set_DAC(void);
uint8_t lowPassFilter(uint8_t currentValue, uint8_t previousValue);

void timer_callback(void *param){

    xN = adc1_get_raw(CHANNEL);
    yN = LPF_update(&LPFilter, (float)xN);
    dac_value = (uint8_t)((yN*255)/4095);
    dac_output_voltage(DAC_CHAN_0, dac_value);
    
    //ESP_LOGI(TAG, "Valor_DAC: %u", dac_value);
}

void app_main(void){
    LPF_init(&LPFilter, FreCort, FreSamp);
    set_ADC();
    set_DAC();
    set_Timer();
    
}

esp_err_t set_ADC(void){
    adc1_config_channel_atten(CHANNEL, ADC_ATTEN_DB_11);
    adc1_config_width(ADC_WIDTH_BIT_12);
    return ESP_OK;
}

esp_err_t set_Timer(void){
   const esp_timer_create_args_t my_timer_args = {
      .callback = &timer_callback,
      .name = "My Timer"};
  esp_timer_handle_t timer_handler;
  ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
  ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, period)); 
  
    return ESP_OK;
}

esp_err_t set_DAC(void){
    dac_output_enable(DAC_CHAN_0);
    return ESP_OK;
}

