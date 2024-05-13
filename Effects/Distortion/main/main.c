/*
 Distorsion
 Comentarios...
*/

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_adc/adc_continuous.h"
#include "driver/gpio.h"
#include "driver/dac.h"

#define CHANNEL1 ADC_CHANNEL_4
#define CHANNEL2 ADC_CHANNEL_5

#define CHANNEL_NUM 2
#define READ_LEN 4

#define LED 5

#define MIN_DISTOR 1000
#define MAX_DISTOR 1800

#define MAX_ADC 4095
#define MIN_ADC 0

//LED
uint8_t toggle = 0;

//ADC
static adc_channel_t channel[CHANNEL_NUM] = {CHANNEL1, CHANNEL2};
adc_continuous_handle_t handlerADC;
uint8_t result[READ_LEN] = {0};
uint32_t ret_num = 0;
uint16_t ADC_Val_1 = 0;
uint16_t ADC_Val_2 = 0;
uint16_t min = 0;
uint16_t max = 0;
uint16_t temp = 0;
uint16_t calib = 0;

//LOG
static const char *TAG = "DISTOR";

//TIMERS
uint64_t period = 100;

//DAC
uint8_t dac_value = 0;

//Distorsion
uint16_t threshold;

//--------------------------------------------------------------------

esp_err_t setADC_DMA(uint8_t channel_num, adc_channel_t *channel);
esp_err_t set_Timer(void);
esp_err_t setLED(void);
esp_err_t set_DAC(void);

//RUTINA PRINCIPAL
void timer_callback(void *param){

        threshold = ((ADC_Val_2-MIN_ADC)/(MAX_ADC-MIN_ADC))*(MAX_DISTOR)+(MIN_DISTOR);
        //dac_value = ((ADC_Val_1-min)/(max-min))*(255)+(0);
        temp = ADC_Val_1;
        if(temp < threshold){ //Entre 1000 y 1800
            temp = threshold;
        }else if(temp > 2900){
            temp = 2900;
        }
        dac_value = (temp*255)/4095;
        dac_output_voltage(DAC_CHAN_0, dac_value);
    

    //ESP_LOGI(TAG, "Min: %u, Max: %u ", min, max);

}

void app_main(void){
   setLED();
   setADC_DMA(CHANNEL_NUM, channel);
   set_DAC();
   set_Timer();
   min = ADC_Val_1;
   max = ADC_Val_1;
}

void ADC_Callback(void){
    ESP_ERROR_CHECK(adc_continuous_read(handlerADC, result, READ_LEN, &ret_num, 1));
    for(size_t i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES) {
        adc_digi_output_data_t *out = (adc_digi_output_data_t*)&result[i];
        if(out->type1.channel == CHANNEL1){
            ADC_Val_1 = out->type1.data;
        }else{
            ADC_Val_2 = out->type1.data;
        }  
    }
}

esp_err_t set_Timer(void){
   const esp_timer_create_args_t my_timer_args = {
      .callback = &timer_callback,
      .name = "Muestreo"};
  esp_timer_handle_t timer_handler;
  ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
  ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, period)); 
  
    return ESP_OK;
}

esp_err_t setADC_DMA(uint8_t channel_num, adc_channel_t *channel){
    
    adc_continuous_handle_cfg_t initDriver = {
        .max_store_buf_size = 4,
        .conv_frame_size = READ_LEN
    };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&initDriver, &handlerADC));


    adc_digi_pattern_config_t patternADC[SOC_ADC_PATT_LEN_MAX] = {0};

    for(uint8_t i = 0; i < channel_num; i++){
        patternADC[i].atten = ADC_ATTEN_DB_11;
        patternADC[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
        patternADC[i].channel = channel[i] & 0x7;
        patternADC[i].unit = ADC_UNIT_1;
    }

    adc_continuous_config_t configADC = {
        .adc_pattern = patternADC,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
        .pattern_num = channel_num,
        .sample_freq_hz = 80000
    };

    ESP_ERROR_CHECK(adc_continuous_config(handlerADC, &configADC));

    adc_continuous_evt_cbs_t configCb = {
        .on_conv_done = ADC_Callback
    };

    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handlerADC, &configCb, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handlerADC));

    return ESP_OK;
}


esp_err_t set_DAC(void){
    dac_output_enable(DAC_CHAN_0);
    return ESP_OK;
}

esp_err_t setLED(void){
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    return ESP_OK;
}