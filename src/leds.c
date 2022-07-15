#include <stdint.h>

#include "NUC123.h"

#define LED_COUNT 26
// 26 leds * 24 bits / 8 bpw
uint8_t led_buffer[LED_COUNT * 3]; //78 bytes

typedef struct
{
	uint8_t green;
	uint8_t red;
	uint8_t blue;
}color_t;

const uint32_t led_lookup[256] = 
{
	0x88888888, 0x8888888E, 0x888888E8, 0x888888EE, 0x88888E88, 0x88888E8E, 0x88888EE8, 0x88888EEE,
	0x8888E888, 0x8888E88E, 0x8888E8E8, 0x8888E8EE, 0x8888EE88, 0x8888EE8E, 0x8888EEE8, 0x8888EEEE,
	0x888E8888, 0x888E888E, 0x888E88E8, 0x888E88EE, 0x888E8E88, 0x888E8E8E, 0x888E8EE8, 0x888E8EEE,
	0x888EE888, 0x888EE88E, 0x888EE8E8, 0x888EE8EE, 0x888EEE88, 0x888EEE8E, 0x888EEEE8, 0x888EEEEE,
	0x88E88888, 0x88E8888E, 0x88E888E8, 0x88E888EE, 0x88E88E88, 0x88E88E8E, 0x88E88EE8, 0x88E88EEE,
	0x88E8E888, 0x88E8E88E, 0x88E8E8E8, 0x88E8E8EE, 0x88E8EE88, 0x88E8EE8E, 0x88E8EEE8, 0x88E8EEEE,
	0x88EE8888, 0x88EE888E, 0x88EE88E8, 0x88EE88EE, 0x88EE8E88, 0x88EE8E8E, 0x88EE8EE8, 0x88EE8EEE,
	0x88EEE888, 0x88EEE88E, 0x88EEE8E8, 0x88EEE8EE, 0x88EEEE88, 0x88EEEE8E, 0x88EEEEE8, 0x88EEEEEE,
	//limit color intencity to 64
	0x8E888888, 0x8E88888E, 0x8E8888E8, 0x8E8888EE, 0x8E888E88, 0x8E888E8E, 0x8E888EE8, 0x8E888EEE,
	0x8E88E888, 0x8E88E88E, 0x8E88E8E8, 0x8E88E8EE, 0x8E88EE88, 0x8E88EE8E, 0x8E88EEE8, 0x8E88EEEE,
	0x8E8E8888, 0x8E8E888E, 0x8E8E88E8, 0x8E8E88EE, 0x8E8E8E88, 0x8E8E8E8E, 0x8E8E8EE8, 0x8E8E8EEE,
	0x8E8EE888, 0x8E8EE88E, 0x8E8EE8E8, 0x8E8EE8EE, 0x8E8EEE88, 0x8E8EEE8E, 0x8E8EEEE8, 0x8E8EEEEE,
	0x8EE88888, 0x8EE8888E, 0x8EE888E8, 0x8EE888EE, 0x8EE88E88, 0x8EE88E8E, 0x8EE88EE8, 0x8EE88EEE,
	0x8EE8E888, 0x8EE8E88E, 0x8EE8E8E8, 0x8EE8E8EE, 0x8EE8EE88, 0x8EE8EE8E, 0x8EE8EEE8, 0x8EE8EEEE,
	0x8EEE8888, 0x8EEE888E, 0x8EEE88E8, 0x8EEE88EE, 0x8EEE8E88, 0x8EEE8E8E, 0x8EEE8EE8, 0x8EEE8EEE,
	0x8EEEE888, 0x8EEEE88E, 0x8EEEE8E8, 0x8EEEE8EE, 0x8EEEEE88, 0x8EEEEE8E, 0x8EEEEEE8, 0x8EEEEEEE,
	// limit color intensity to 128
	0xE8888888, 0xE888888E, 0xE88888E8, 0xE88888EE, 0xE8888E88, 0xE8888E8E, 0xE8888EE8, 0xE8888EEE,
	0xE888E888, 0xE888E88E, 0xE888E8E8, 0xE888E8EE, 0xE888EE88, 0xE888EE8E, 0xE888EEE8, 0xE888EEEE,
	0xE88E8888, 0xE88E888E, 0xE88E88E8, 0xE88E88EE, 0xE88E8E88, 0xE88E8E8E, 0xE88E8EE8, 0xE88E8EEE,
	0xE88EE888, 0xE88EE88E, 0xE88EE8E8, 0xE88EE8EE, 0xE88EEE88, 0xE88EEE8E, 0xE88EEEE8, 0xE88EEEEE,
	0xE8E88888, 0xE8E8888E, 0xE8E888E8, 0xE8E888EE, 0xE8E88E88, 0xE8E88E8E, 0xE8E88EE8, 0xE8E88EEE,
	0xE8E8E888, 0xE8E8E88E, 0xE8E8E8E8, 0xE8E8E8EE, 0xE8E8EE88, 0xE8E8EE8E, 0xE8E8EEE8, 0xE8E8EEEE,
	0xE8EE8888, 0xE8EE888E, 0xE8EE88E8, 0xE8EE88EE, 0xE8EE8E88, 0xE8EE8E8E, 0xE8EE8EE8, 0xE8EE8EEE,
	0xE8EEE888, 0xE8EEE88E, 0xE8EEE8E8, 0xE8EEE8EE, 0xE8EEEE88, 0xE8EEEE8E, 0xE8EEEEE8, 0xE8EEEEEE,
	
	0xEE888888, 0xEE88888E, 0xEE8888E8, 0xEE8888EE, 0xEE888E88, 0xEE888E8E, 0xEE888EE8, 0xEE888EEE,
	0xEE88E888, 0xEE88E88E, 0xEE88E8E8, 0xEE88E8EE, 0xEE88EE88, 0xEE88EE8E, 0xEE88EEE8, 0xEE88EEEE,
	0xEE8E8888, 0xEE8E888E, 0xEE8E88E8, 0xEE8E88EE, 0xEE8E8E88, 0xEE8E8E8E, 0xEE8E8EE8, 0xEE8E8EEE,
	0xEE8EE888, 0xEE8EE88E, 0xEE8EE8E8, 0xEE8EE8EE, 0xEE8EEE88, 0xEE8EEE8E, 0xEE8EEEE8, 0xEE8EEEEE,
	0xEEE88888, 0xEEE8888E, 0xEEE888E8, 0xEEE888EE, 0xEEE88E88, 0xEEE88E8E, 0xEEE88EE8, 0xEEE88EEE,
	0xEEE8E888, 0xEEE8E88E, 0xEEE8E8E8, 0xEEE8E8EE, 0xEEE8EE88, 0xEEE8EE8E, 0xEEE8EEE8, 0xEEE8EEEE,
	0xEEEE8888, 0xEEEE888E, 0xEEEE88E8, 0xEEEE88EE, 0xEEEE8E88, 0xEEEE8E8E, 0xEEEE8EE8, 0xEEEE8EEE,
	0xEEEEE888, 0xEEEEE88E, 0xEEEEE8E8, 0xEEEEE8EE, 0xEEEEEE88, 0xEEEEEE8E, 0xEEEEEEE8, 0xEEEEEEEE
};

color_t rgb2c(uint8_t red, uint8_t green, uint8_t blue) {
	color_t c;
	c.red = red;
	c.green = green;
	c.blue = blue;
	return c;
}

color_t hsv2c(uint8_t hue, uint8_t saturation, uint8_t value)
{
	color_t c;
	uint32_t red, green, blue;
	if (hue < 86)
	{
		uint8_t h = hue;
		red = (85 - h) * 771;
		green = h * 771;
		blue = 0;
	}
	else
	{
		if (hue < 171)
		{
			uint8_t h = hue - 85;
			red = 0;
			green = (85 - h) * 771;
			blue = h * 771;
		}
		else
		{
			uint8_t h = hue - 170;
			red = h * 771;
			green = 0;
			blue = (85 - h) * 771;
		}
	}
	if (red > 32768) red = 32768;
	if (green > 32768) green = 32768;
	if (blue > 32768) blue = 32768;
	//saturation
	red = red*saturation/256 + (256-saturation)*64;
	green = green*saturation/256 + (256-saturation)*64;
	blue = blue*saturation/256 + (256-saturation)*64;
	//value
	if (value > 128)
	{ //to bright
		value = value - 128;
		red = red*(128-value)/128 + value*256;
		green = green*(128-value)/128 + value*256;
		blue = blue*(128-value)/128 + value*256;
	}
	else
	{ //to dark
		red = red*value/128;
		green = green*value/128;
		blue = blue*value/128;
	}
	c.red = red >> 7;
	c.green = green >> 7;
	c.blue = blue >> 7;
	return c;
}


void led_set(uint32_t led, color_t color)
{
	//led = led > LED_COUNT ? 26 : led;
	// BRG?
	color_t *c = (color_t*)&led_buffer[led * 3];
	*c = color;
}

void led_init()
{
	//init spi
	SPI1->DIVIDER = 0xA; //0x00000015;
	SPI1->CNTRL = SPI_CNTRL_TWOB_Msk;
	SPI1->CNTRL2 = SPI_CNTRL2_DUAL_IO_DIR_Msk;
	SPI1->CNTRL |= SPI_CNTRL_FIFO_Msk;
	//SPI1->CNTRL = 0x052000000;
	//init dma

	/*
	PDMA_GCR->GCRCSR = 0x00000100;
	PDMA_GCR->PDSSR0 = 0x00FF0FFF;
	PDMA0->CSR = 0x00000089;
	PDMA0->SAR = (uint32_t)led_buffer;
	PDMA0->DAR = (uint32_t)&SPI1->TX[1];
	PDMA0->BCR = 0x24; // sizeof(led_buffer); !!
	PDMA0->IER = 0;
	// enable channel
	PDMA0->CSR |= PDMA_CSR_TRIG_EN_Msk;
	// start spi
	
	//SPI1->TX[0] = 0x5500FFAA;
	//SPI1->CNTRL |= SPI_CNTRL_GO_BUSY_Msk;
	//SPI1->DMA |= SPI_DMA_TX_DMA_GO_Msk;
	//led_update();
	SPI2->DIVIDER = 0xA;
	SPI2->CNTRL = 0;
	SPI2->CNTRL2 = SPI_CNTRL2_DUAL_IO_DIR_Msk;
	SPI2->CNTRL |= SPI_CNTRL_FIFO_Msk;
	*/

	//init pwm
	PWMA->PPR = 0x00000200; // only oscilloscope can help...
	PWMA->CSR = 0x00004333; // datasheet timings are not correct, or i am dumb
	PWMA->PCR = PWM_PCR_CH3PINV_Msk; //PWM_PCR_CH3MOD_Msk | 
	// N = 7 M = 7 - constant 0
	// N = 7 M = 5 - 6/8 - logic 1
	// N = 7 M = 2 - 3/8 - logic 0
	// N = 7 M = 0 - 1/8
	PWMA->POE = PWM_POE_PWM3_Msk;
	//PWMA->CAPENR = 0x00000008;
	//start counter
	PWMA->PCR |= PWM_PCR_CH3EN_Msk;
	//PWMA->CMR3 = PL_PERIOD; // keep low
	//PWMA->CNR3 = PL_PERIOD;
	
}

/*
void ddelay_us(volatile uint32_t time_us)
{
	//calc us
	uint32_t time = 72UL * time_us / 4;
	while(time--);
}
*/
/*
void led_update()
{
	//ddelay_us(1000000);
	wait_one_sec();
	while(SPI1->DMA & SPI_DMA_TX_DMA_GO_Msk);
	SPI1->DMA |= SPI_DMA_PDMA_RST_Msk;
	while(SPI1->DMA & SPI_DMA_PDMA_RST_Msk);
	PDMA0->CSR |= PDMA_CSR_SW_RST_Msk;
	while(PDMA0->CSR & PDMA_CSR_SW_RST_Msk);
	PDMA0->CSR |= PDMA_CSR_TRIG_EN_Msk;
	SPI1->DMA |= SPI_DMA_TX_DMA_GO_Msk;
	
}
*/

#define PL_PERIOD 7
#define PL_ONE    4 // 3
#define PL_ZERO   6 // 1


void led_scan_tick(uint32_t src) {
	// 256 cycles
	uint32_t pos = (uint8_t)src;

	// update pads - 
	if (pos < 16 * 3) {
		//send led data
		uint32_t lval = led_lookup[led_buffer[pos]];
		SPI1->TX[1] = 0;
		SPI1->TX[1] = lval;
		// SPI2->TX[0] = lval;
	}

	// update function buttons 240 bits
	if (pos < 10 * 24) {
		uint32_t led_word = led_buffer[16 * 3 + pos / 8];
		PWMA->CMR3 = (led_word << (pos & 0x7)) & 0x80 ? PL_ONE : PL_ZERO;
		PWMA->CNR3 = PL_PERIOD;
	}
	
}

inline void pwm_bit(uint32_t bit) {
	//wait
	while(!(PWMA->PIIR & PWM_PIIR_PWMIF3_Msk));
	//clear flags
	PWMA->PIIR = PWM_PIIR_PWMDIF3_Msk | PWM_PIIR_PWMIF3_Msk;
	PWMA->CMR3 = bit ? PL_ONE : PL_ZERO;
	PWMA->CNR3 = PL_PERIOD;
}



/*
void led_init2()
{
	//init pwm
	PWMA->PPR = 0x00000200; // only oscilloscope can help...
	PWMA->CSR = 0x00004333; // datasheet timings are not correct, or i am dumb
	PWMA->PCR = PWM_PCR_CH3PINV_Msk; //PWM_PCR_CH3MOD_Msk | 
	// N = 7 M = 7 - constant 0
	// N = 7 M = 5 - 6/8 - logic 1
	// N = 7 M = 2 - 3/8 - logic 0
	// N = 7 M = 0 - 1/8
	PWMA->POE = PWM_POE_PWM3_Msk;
	//PWMA->CAPENR = 0x00000008;
	//start counter
	PWMA->PCR |= PWM_PCR_CH3EN_Msk;
	//PWMA->CMR3 = PL_PERIOD; // keep low
	//PWMA->CNR3 = PL_PERIOD;
	
	return;
	
	
while(1){
	for (uint32_t i_led = 0; i_led < 8 * 3; i_led++)
	{
		uint32_t led = led_buffer[i_led];
		for (uint32_t i_bit = 0; i_bit < 8; i_bit++)
		{
			ddelay_us(21); // 96 kHz
			//wait cycle ends
			pwm_bit((led << i_bit) & 0x80);
		}
	}
	ddelay_us(100000); // 96 kHz
}
	while(1);
	
	//init dma
	
	PDMA_GCR->GCRCSR = 0x00000100;
	PDMA_GCR->PDSSR0 = 0x00FFFFFF;
	PDMA_GCR->PDSSR1 = 0x00FFFFFF;
	PDMA_GCR->PDSSR2 = 0x000FFFFF;
	PDMA0->CSR = 0x00080089;
	PDMA0->SAR = (uint32_t)ledcolor;
	PDMA0->DAR = (uint32_t)&PWMA->CMR3;
	PDMA0->BCR = sizeof(ledcolor);
	PDMA0->IER = 0;
	// enable channel
	PDMA0->CSR |= PDMA_CSR_TRIG_EN_Msk;
	
	
}
*/

/*
void led_scan_tick2(uint32_t src) {
	// 10 * 24 = 240 (256)
	uint32_t bit_pos = (uint8_t)src;
	if (bit_pos < 10 * 24) {
		uint32_t led_word = led_buffer[16 * 3 + bit_pos / 8];
		PWMA->CMR3 = (led_word << (bit_pos & 0x7)) & 0x80 ? PL_ONE : PL_ZERO;
		PWMA->CNR3 = PL_PERIOD;
	}
}

*/