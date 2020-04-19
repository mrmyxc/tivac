#ifndef HAL_ADC_H__
#define HAL_ADC_H__

typedef enum adcret
{
    ADC_OK,
    ADC_ERR
};

enum adcret adc_init( void );

#endif