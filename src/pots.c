
#include "NUC123.h"

volatile uint32_t start;
volatile uint32_t stop;

void adcInit()
{
    PA->DOUT = 0;
    // clk: adc source: 12MHz, div2
    ADC->ADCR = ADC_ADCR_ADEN_Msk | ADC_ADCR_ADMD_SINGLE_CYCLE;
    ADC->ADCHER = 0x0000003F;
    ADC->ADCR |= ADC_ADCR_ADST_Msk;
    start = TIMER0->TDR;
    while (ADC->ADCR & ADC_ADCR_ADST_Msk)
        ;
    stop = TIMER0->TDR;
}

volatile int32_t adc_pitchwheel;
volatile int32_t adc_modwheel;

volatile int16_t adc_knob[8];
volatile uint16_t adc_pad[16];

// ADC is 0 to 0x3FF - need additional 4 bits
#define SENSOR_CENTER 0x240
#define SENSOR_SIZE 0x155
#define SENSOR_ACTIVE_THRSH (SENSOR_CENTER - (0x400 - SENSOR_CENTER)) // 0x080
#define SENSOR_FILTER 256
#define SENSOR_RAND_PS ((int32_t)(0x100000000LL / (SENSOR_FILTER * 2)))

#define PAD_THRSH (0x400 - 64)

void adcSrTap(uint32_t sr, uint32_t lcg)
{
    // TODO: division ????
    if ((sr & 0x3) != 0)
        return;
    else {
        sr = sr >> 2;
        // switch to next
        if (ADC->ADCR & ADC_ADCR_ADST_Msk)
            while (1)
                ;
        PA->DOUT = (sr + 1) << 12;

        int32_t pitch = (int16_t)ADC->ADDR[0];
        if (pitch < SENSOR_ACTIVE_THRSH)
            pitch = SENSOR_CENTER;
        static int32_t flt_pitchwheel = 0;
        flt_pitchwheel += (pitch * SENSOR_FILTER - flt_pitchwheel + lcg / SENSOR_RAND_PS) / SENSOR_FILTER;
		pitch = (flt_pitchwheel - (SENSOR_CENTER * SENSOR_FILTER)) * (0x2000 / SENSOR_SIZE) / SENSOR_FILTER;
		if (pitch < -0x1FFF) pitch = -0x1FFF;
		else if (pitch > 0x2000) pitch = 0x2000;
		pitch = 0x2000 - pitch;
        adc_pitchwheel = pitch;

        int32_t mod = (int16_t)ADC->ADDR[1];
        if (mod > SENSOR_ACTIVE_THRSH) {
        	static int32_t flt_modwheel = 0;
            flt_modwheel += (mod * SENSOR_FILTER - flt_modwheel + lcg / SENSOR_RAND_PS) / SENSOR_FILTER;
			mod = (flt_modwheel - (SENSOR_CENTER * SENSOR_FILTER)) * (0x2000 / SENSOR_SIZE) / SENSOR_FILTER;
			if (mod < -0x1FFF) mod = -0x1FFF;
			else if (mod > 0x2000) mod = 0x2000;
			mod = 0x2000 - mod;
        	adc_modwheel = mod;
        }

        uint32_t pos = sr & 0x7;

        adc_knob[pos] += (((int16_t)ADC->ADDR[3]) * 16 - adc_knob[pos] + lcg / 0x8000000) / 16;

        adc_pad[pos] = ADC->ADDR[2];
        adc_pad[pos + 8] = ADC->ADDR[4];
        ADC->ADCR |= ADC_ADCR_ADST_Msk;
    }
}

/*
adc 0..0x400
f 0..0x2000
*/

/*
static inline uint32_t sensorIsActive(uint16_t adc_in, uint16_t result_out){
        if (adc_in > SENSOR_ACTIVE_THRSH) {
                result_out = 0;
        }
        return 0;
}
*/
