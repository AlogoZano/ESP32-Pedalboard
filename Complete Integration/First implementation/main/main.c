/*
 Integracion 1
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
#include "ssd1306.h"

#define CHANNEL1 ADC_CHANNEL_4
#define CHANNEL2 ADC_CHANNEL_5

#define CHANNEL_NUM 2
#define READ_LEN 4

#define INT1 23
#define INT2 34

#define LED 5

#define NUM_EFECTOS 3

#define MAX_ADC 4095
#define MIN_ADC 0

#define MIN_DISTOR 1000
#define MAX_DISTOR 1800

#define MIN_TREMOLO 1
#define MAX_TREMOLO 4

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
static const char *TAG = "Completo1";

//TIMERS
uint64_t period = 180;

//DAC
uint8_t dac_value = 0;

//OLED
SSD1306_t dev;
uint8_t selectorEffect = 0;
uint8_t okEfecto = 0;

//Multicore
TaskHandle_t Interfaz;

//Distorsion
uint16_t threshold = 0; //Valor inicial de clippeado de distorsion

//Tremolo
const uint8_t waveform[]=
{
0x0,0x1,0x1,0x2,0x2,0x2,0x3,0x3,0x4,0x4,0x4,0x5,0x5,0x5,0x6,0x6,0x7,0x7,0x7,0x8,
0x8,0x9,0x9,0x9,0xa,0xa,0xb,0xb,0xb,0xc,0xc,0xd,0xd,0xd,0xe,0xe,0xe,0xf,0xf,0x10,
0x10,0x10,0x11,0x11,0x12,0x12,0x12,0x13,0x13,0x14,0x14,0x14,0x15,0x15,0x16,0x16,0x16,0x17,0x17,0x17,
0x18,0x18,0x19,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1b,0x1c,0x1c,0x1d,0x1d,0x1d,0x1e,0x1e,0x1e,0x1f,0x1f,
0x20,0x20,0x20,0x21,0x21,0x22,0x22,0x22,0x23,0x23,0x24,0x24,0x24,0x25,0x25,0x26,0x26,0x26,0x27,0x27,
0x27,0x28,0x28,0x29,0x29,0x29,0x2a,0x2a,0x2b,0x2b,0x2b,0x2c,0x2c,0x2d,0x2d,0x2d,0x2e,0x2e,0x2f,0x2f,
0x2f,0x30,0x30,0x30,0x31,0x31,0x32,0x32,0x32,0x33,0x33,0x34,0x34,0x34,0x35,0x35,0x36,0x36,0x36,0x37,
0x37,0x38,0x38,0x38,0x39,0x39,0x39,0x3a,0x3a,0x3b,0x3b,0x3b,0x3c,0x3c,0x3d,0x3d,0x3d,0x3e,0x3e,0x3f,
0x3f,0x3f,0x40,0x40,0x41,0x41,0x41,0x42,0x42,0x42,0x43,0x43,0x44,0x44,0x44,0x45,0x45,0x46,0x46,0x46,
0x47,0x47,0x48,0x48,0x48,0x49,0x49,0x4a,0x4a,0x4a,0x4b,0x4b,0x4b,0x4c,0x4c,0x4d,0x4d,0x4d,0x4e,0x4e,
0x4f,0x4f,0x4f,0x50,0x50,0x51,0x51,0x51,0x52,0x52,0x53,0x53,0x53,0x54,0x54,0x54,0x55,0x55,0x56,0x56,
0x56,0x57,0x57,0x58,0x58,0x58,0x59,0x59,0x5a,0x5a,0x5a,0x5b,0x5b,0x5b,0x5c,0x5c,0x5d,0x5d,0x5d,0x5e,
0x5e,0x5f,0x5f,0x5f,0x60,0x60,0x61,0x61,0x61,0x62,0x62,0x63,0x63,0x63,0x64,0x64,0x64,0x65,0x65,0x66,
0x66,0x66,0x67,0x67,0x68,0x68,0x68,0x69,0x69,0x6a,0x6a,0x6a,0x6b,0x6b,0x6c,0x6c,0x6c,0x6d,0x6d,0x6d,
0x6e,0x6e,0x6f,0x6f,0x6f,0x70,0x70,0x71,0x71,0x71,0x72,0x72,0x73,0x73,0x73,0x74,0x74,0x75,0x75,0x75,
0x76,0x76,0x76,0x77,0x77,0x78,0x78,0x78,0x79,0x79,0x7a,0x7a,0x7a,0x7b,0x7b,0x7c,0x7c,0x7c,0x7d,0x7d,
0x7e,0x7e,0x7e,0x7f,0x7f,0x7f,0x80,0x80,0x81,0x81,0x81,0x82,0x82,0x83,0x83,0x83,0x84,0x84,0x85,0x85,
0x85,0x86,0x86,0x87,0x87,0x87,0x88,0x88,0x88,0x89,0x89,0x8a,0x8a,0x8a,0x8b,0x8b,0x8c,0x8c,0x8c,0x8d,
0x8d,0x8e,0x8e,0x8e,0x8f,0x8f,0x8f,0x90,0x90,0x91,0x91,0x91,0x92,0x92,0x93,0x93,0x93,0x94,0x94,0x95,
0x95,0x95,0x96,0x96,0x97,0x97,0x97,0x98,0x98,0x98,0x99,0x99,0x9a,0x9a,0x9a,0x9b,0x9b,0x9c,0x9c,0x9c,
0x9d,0x9d,0x9e,0x9e,0x9e,0x9f,0x9f,0xa0,0xa0,0xa0,0xa1,0xa1,0xa1,0xa2,0xa2,0xa3,0xa3,0xa3,0xa4,0xa4,
0xa5,0xa5,0xa5,0xa6,0xa6,0xa7,0xa7,0xa7,0xa8,0xa8,0xa9,0xa9,0xa9,0xaa,0xaa,0xaa,0xab,0xab,0xac,0xac,
0xac,0xad,0xad,0xae,0xae,0xae,0xaf,0xaf,0xb0,0xb0,0xb0,0xb1,0xb1,0xb2,0xb2,0xb2,0xb3,0xb3,0xb3,0xb4,
0xb4,0xb5,0xb5,0xb5,0xb6,0xb6,0xb7,0xb7,0xb7,0xb8,0xb8,0xb9,0xb9,0xb9,0xba,0xba,0xbb,0xbb,0xbb,0xbc,
0xbc,0xbc,0xbd,0xbd,0xbe,0xbe,0xbe,0xbf,0xbf,0xc0,0xc0,0xc0,0xc1,0xc1,0xc2,0xc2,0xc2,0xc3,0xc3,0xc4,
0xc4,0xc4,0xc5,0xc5,0xc5,0xc6,0xc6,0xc7,0xc7,0xc7,0xc8,0xc8,0xc8,0xc7,0xc7,0xc7,0xc6,0xc6,0xc5,0xc5,
0xc5,0xc4,0xc4,0xc4,0xc3,0xc3,0xc2,0xc2,0xc2,0xc1,0xc1,0xc0,0xc0,0xc0,0xbf,0xbf,0xbe,0xbe,0xbe,0xbd,
0xbd,0xbc,0xbc,0xbc,0xbb,0xbb,0xbb,0xba,0xba,0xb9,0xb9,0xb9,0xb8,0xb8,0xb7,0xb7,0xb7,0xb6,0xb6,0xb5,
0xb5,0xb5,0xb4,0xb4,0xb3,0xb3,0xb3,0xb2,0xb2,0xb2,0xb1,0xb1,0xb0,0xb0,0xb0,0xaf,0xaf,0xae,0xae,0xae,
0xad,0xad,0xac,0xac,0xac,0xab,0xab,0xaa,0xaa,0xaa,0xa9,0xa9,0xa9,0xa8,0xa8,0xa7,0xa7,0xa7,0xa6,0xa6,
0xa5,0xa5,0xa5,0xa4,0xa4,0xa3,0xa3,0xa3,0xa2,0xa2,0xa1,0xa1,0xa1,0xa0,0xa0,0xa0,0x9f,0x9f,0x9e,0x9e,
0x9e,0x9d,0x9d,0x9c,0x9c,0x9c,0x9b,0x9b,0x9a,0x9a,0x9a,0x99,0x99,0x98,0x98,0x98,0x97,0x97,0x97,0x96,
0x96,0x95,0x95,0x95,0x94,0x94,0x93,0x93,0x93,0x92,0x92,0x91,0x91,0x91,0x90,0x90,0x8f,0x8f,0x8f,0x8e,
0x8e,0x8e,0x8d,0x8d,0x8c,0x8c,0x8c,0x8b,0x8b,0x8a,0x8a,0x8a,0x89,0x89,0x88,0x88,0x88,0x87,0x87,0x87,
0x86,0x86,0x85,0x85,0x85,0x84,0x84,0x83,0x83,0x83,0x82,0x82,0x81,0x81,0x81,0x80,0x80,0x7f,0x7f,0x7f,
0x7e,0x7e,0x7e,0x7d,0x7d,0x7c,0x7c,0x7c,0x7b,0x7b,0x7a,0x7a,0x7a,0x79,0x79,0x78,0x78,0x78,0x77,0x77,
0x76,0x76,0x76,0x75,0x75,0x75,0x74,0x74,0x73,0x73,0x73,0x72,0x72,0x71,0x71,0x71,0x70,0x70,0x6f,0x6f,
0x6f,0x6e,0x6e,0x6d,0x6d,0x6d,0x6c,0x6c,0x6c,0x6b,0x6b,0x6a,0x6a,0x6a,0x69,0x69,0x68,0x68,0x68,0x67,
0x67,0x66,0x66,0x66,0x65,0x65,0x64,0x64,0x64,0x63,0x63,0x63,0x62,0x62,0x61,0x61,0x61,0x60,0x60,0x5f,
0x5f,0x5f,0x5e,0x5e,0x5d,0x5d,0x5d,0x5c,0x5c,0x5b,0x5b,0x5b,0x5a,0x5a,0x5a,0x59,0x59,0x58,0x58,0x58,
0x57,0x57,0x56,0x56,0x56,0x55,0x55,0x54,0x54,0x54,0x53,0x53,0x53,0x52,0x52,0x51,0x51,0x51,0x50,0x50,
0x4f,0x4f,0x4f,0x4e,0x4e,0x4d,0x4d,0x4d,0x4c,0x4c,0x4b,0x4b,0x4b,0x4a,0x4a,0x4a,0x49,0x49,0x48,0x48,
0x48,0x47,0x47,0x46,0x46,0x46,0x45,0x45,0x44,0x44,0x44,0x43,0x43,0x42,0x42,0x42,0x41,0x41,0x41,0x40,
0x40,0x3f,0x3f,0x3f,0x3e,0x3e,0x3d,0x3d,0x3d,0x3c,0x3c,0x3b,0x3b,0x3b,0x3a,0x3a,0x39,0x39,0x39,0x38,
0x38,0x38,0x37,0x37,0x36,0x36,0x36,0x35,0x35,0x34,0x34,0x34,0x33,0x33,0x32,0x32,0x32,0x31,0x31,0x30,
0x30,0x30,0x2f,0x2f,0x2f,0x2e,0x2e,0x2d,0x2d,0x2d,0x2c,0x2c,0x2b,0x2b,0x2b,0x2a,0x2a,0x29,0x29,0x29,
0x28,0x28,0x27,0x27,0x27,0x26,0x26,0x26,0x25,0x25,0x24,0x24,0x24,0x23,0x23,0x22,0x22,0x22,0x21,0x21,
0x20,0x20,0x20,0x1f,0x1f,0x1e,0x1e,0x1e,0x1d,0x1d,0x1d,0x1c,0x1c,0x1b,0x1b,0x1b,0x1a,0x1a,0x19,0x19,
0x19,0x18,0x18,0x17,0x17,0x17,0x16,0x16,0x16,0x15,0x15,0x14,0x14,0x14,0x13,0x13,0x12,0x12,0x12,0x11,
0x11,0x10,0x10,0x10,0xf,0xf,0xe,0xe,0xe,0xd,0xd,0xd,0xc,0xc,0xb,0xb,0xb,0xa,0xa,0x9,
0x9,0x9,0x8,0x8,0x7,0x7,0x7,0x6,0x6,0x5,0x5,0x5,0x4,0x4,0x4,0x3,0x3,0x2,0x2,0x2,
0x1,0x1,0x0,0x0,
};

uint8_t divider = 0;
uint16_t sample = 0;
uint8_t depth = 0;
uint16_t valorNorm = 0;

//---------------------------------------------------------------------------------------------------------

//Set peripherals
esp_err_t setADC_DMA(uint8_t channel_num, adc_channel_t *channel);
esp_err_t set_Timer(void);
esp_err_t setLED(void);
esp_err_t set_DAC(void);
esp_err_t set_OLED(void);
esp_err_t ExtITConfig(); //Está función configura dos pines para interrupción definidos anteriormente

//Set dual core
esp_err_t TaskInterfaz(void);
void set_Interfaz(void *args);

//Interfaz
void updateInterface();

//Callbacks
void ADC_Callback(void *args);
void timer_callback(void *args);
void ExtISR_1(void *args);
void ExtISR_2(void *args);

//Effectos
uint8_t Clean(uint16_t input);
uint8_t Distorsion(uint16_t input, uint16_t param);
uint8_t Tremolo(uint16_t input, uint16_t param);
uint8_t LPF(uint16_t input, uint16_t param);

//---------------------------------------------------------------------------------------------------------

//RUTINA PRINCIPAL CPU0
void timer_callback(void *args){

    if((selectorEffect == 1) && (okEfecto != 0)){
        dac_output_voltage(DAC_CHAN_0, Distorsion(ADC_Val_1, ADC_Val_2));

    }else if((selectorEffect == 2) && (okEfecto != 0)){
        dac_output_voltage(DAC_CHAN_0, Tremolo(ADC_Val_1, ADC_Val_2));

    }else{
        dac_output_voltage(DAC_CHAN_0, Clean(ADC_Val_1));
    }

    //dac_value = ((ADC_Val_1-min)/(max-min))*(255)+(0);
    //dac_value = (ADC_Val_1*255)/4095;
    //dac_output_voltage(DAC_CHAN_0, dac_value);
    //updateInterface();

    //ESP_LOGI(TAG, "Val: %u", ADC_Val_2);

}

//RUTINA PRINCIPAL CPU1
void set_Interfaz(void *args){
    ESP_ERROR_CHECK(set_OLED());
    ESP_ERROR_CHECK(ExtITConfig());
    ssd1306_contrast(&dev, 0xff);
	ssd1306_clear_screen(&dev, false);
    
    char buff[50];
    while(1){
        
        updateInterface();

    }
}


void app_main(void){
   
   ESP_ERROR_CHECK(setLED());
   ESP_ERROR_CHECK(setADC_DMA(CHANNEL_NUM, channel));
   ESP_ERROR_CHECK(set_DAC());
   ESP_ERROR_CHECK(set_Timer());
   ESP_ERROR_CHECK(TaskInterfaz());
}

uint8_t Clean(uint16_t input){
    return (uint8_t)((input*255)/4095);
}

uint8_t Distorsion(uint16_t input, uint16_t param){
    threshold = ((param-MIN_ADC)/(MAX_ADC-MIN_ADC))*(MAX_DISTOR)+(MIN_DISTOR);

    if(input < threshold){ //Entre 1000 y 1800
        input = 1000;
    }else if(input > 3000){
        input = 3000;
    }
    
    return (uint8_t)((input*255)/4095);
}

uint8_t Tremolo(uint16_t input, uint16_t param){

    depth = ((param-MIN_ADC)/(MAX_ADC-MIN_ADC))*(MAX_TREMOLO)+(MIN_TREMOLO);
    valorNorm = (input*(waveform[sample]*20))/4095;
    divider++;
    if (divider==4){ divider=0; sample=sample+depth;}
    if(sample>1023)sample=0;

    return (uint8_t)((valorNorm*255)/4095);

}

void ExtISR_1(void *args){
    selectorEffect++;
    selectorEffect %= NUM_EFECTOS;
}

void ExtISR_2(void *args){
    okEfecto = !okEfecto;
}

esp_err_t ExtITConfig(){
    gpio_config_t GPIOHandler1 = {
        .pin_bit_mask = (1ULL << INT1),
        .mode = GPIO_MODE_DEF_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };

    gpio_config_t GPIOHandler2 = {
        .pin_bit_mask = (1ULL << INT2),
        .mode = GPIO_MODE_DEF_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };

    gpio_config(&GPIOHandler1);
    gpio_config(&GPIOHandler2);

    gpio_install_isr_service(0); //Habilitan globalmente interrupciones

    gpio_isr_handler_add(INT1, ExtISR_1, NULL);
    gpio_isr_handler_add(INT2, ExtISR_2, NULL);

    return ESP_OK;
}

void ADC_Callback(void *args){
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

esp_err_t set_OLED(void){


	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

    
	ssd1306_init(&dev, 128, 64);

    //ssd1306_contrast(&dev, 0xff);
	//ssd1306_clear_screen(&dev, false);

    return ESP_OK;
}

void updateInterface(){
    if((selectorEffect == 0) && (okEfecto == 0)){
        ssd1306_display_text(&dev, 0, " Distorsion", 11, true);
        ssd1306_display_text(&dev, 2, " Tremolo", 8, false);
        ssd1306_display_text(&dev, 4, " LPF", 4, false);

    }else if((selectorEffect == 1) && (okEfecto == 0)){
        ssd1306_display_text(&dev, 0, " Distorsion", 11, false);
        ssd1306_display_text(&dev, 2, " Tremolo", 8, true);
        ssd1306_display_text(&dev, 4, " LPF", 4, false);

    }else if((selectorEffect == 2) && (okEfecto == 0)){
        ssd1306_display_text(&dev, 0, " Distorsion", 11, false);
        ssd1306_display_text(&dev, 2, " Tremolo", 8, false);
        ssd1306_display_text(&dev, 4, " LPF", 4, true);

    }else if((selectorEffect == 0) && (okEfecto != 0)){
        ssd1306_display_text(&dev, 0, " Distorsion", 11, false);
    }

    else if((selectorEffect == 1) && (okEfecto != 0)){
        ssd1306_display_text(&dev, 2, " Tremolo", 8, false);
    }

    else if((selectorEffect == 2) && (okEfecto != 0)){
        ssd1306_display_text(&dev, 4, " LPF", 4, false);
    }
}

esp_err_t TaskInterfaz(void){
    xTaskCreatePinnedToCore(set_Interfaz, "InicioInter", 10000, NULL, 1, &Interfaz, 1);
    return ESP_OK;
}