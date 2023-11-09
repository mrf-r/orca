#include "i2c_proc.h"
// lcd update:
// 128 * 8 bytes = 1024
// 0.0256 s - 40 fps

int i2cMasterTransaction(I2CTransaction* wtr, I2CTransaction* rtr);

#define LCD_FPS 16

#define SSD1306_I2C_ADDRESS 0x78
#define SSD1306_I2CMODE_SINGLE_CMD 0x80
#define SSD1306_I2CMODE_STREAM_CMD 0x00
#define SSD1306_I2CMODE_SINGLE_DAT 0xC0
#define SSD1306_I2CMODE_STREAM_DAT 0x40

static const uint8_t lcd_init_sequence[] = {
    SSD1306_I2CMODE_STREAM_CMD,
    0xAE, // SSD1306_DISPLAYOFF
    0xD5, 0x80, // SSD1306_SETDISPLAYCLOCKDIV 80
    0xA8, 0x3F, // SSD1306_SETMULTIPLEX 3F
    0xD3, 0x00, // SSD1306_SETDISPLAYOFFSET
    0x8D, 0x15, // SSD1306_CHARGEPUMP
    0x40, // SSD1306_SETSTARTLINE
    0xA6, // SSD1306_NORMALDISPLAY
    0xA4, // SSD1306_DISPLAYALLON_RESUME

    0xA1, // SSD1306_SEGREMAP (FLIP) A0
    0xC8, // SSD1306_COMSCANDEC C0
    // 0x20, 0x00, // SSD1306_MEMORYMODE

    0xDA, 0x12, // SSD1306_SETCOMPINS
    0x81, 0x3F, // SSD1306_SETCONTRAST
    0xD9, 0x77, // SSD1306_SETPRECHARGE F1
    0xDB, 0x20, // SSD1306_SETVCOMDETECT 40?
    0xAF // SSD1306_DISPLAYON
};

#define LCD_UPDATE_SEQUENCE_LINE_BYTE_POSITION 1
static const uint8_t lcd_update_sequence[] = {
    SSD1306_I2CMODE_SINGLE_CMD,
    0xB0, // SSD1306_SETPAGESTART
    SSD1306_I2CMODE_SINGLE_CMD,
    0x00, // SSD1306_SETCOLUMNSTARTLOW
    SSD1306_I2CMODE_SINGLE_CMD,
    0x10, // SSD1306_SETCOLUMNSTARTHIGH
    SSD1306_I2CMODE_STREAM_DAT
};

// this called from main
void lcdStart()
{
    I2CTransaction i2ct_lcd_init = {
        .status = 0,
        .address = SSD1306_I2C_ADDRESS,
        .len = sizeof(lcd_init_sequence),
        .data = lcd_init_sequence,
        .complete_callback = 0
    };
    i2cSeqTransaction(&i2ct_lcd_init, 0, 1);
    ASSERT(i2ct_lcd_init.status == ITS_RESULT_OK);
}

static uint8_t lcd_update_ctrl_buff[sizeof(lcd_update_sequence)];
static I2CTransaction i2ct_lcd_update;

extern uint8_t mgl_framebuffer[128 * 8];
static uint8_t line;

static I2CTransaction* lcdUpdateDataCallback()
{
    ASSERT(i2ct_lcd_update.status & ITS_COMPLETE);
    i2ct_lcd_update.status = 0;
    i2ct_lcd_update.len = 128;
    i2ct_lcd_update.data = &mgl_framebuffer[128 * line];
    i2ct_lcd_update.complete_callback = 0;
    return &i2ct_lcd_update;
}

// to be called when bus is empty
// returns true if transaction was initiated
bool i2cSeqLcd()
{
    static uint32_t next_update = 0;
    if (((int32_t)next_update - (int32_t)counter_sr) < 0) {
        // start transaction
        line = (line + 1) & 0x7;
        for (int i = 0; i < sizeof(lcd_update_sequence); i++) {
            lcd_update_ctrl_buff[i] = lcd_update_sequence[i];
        }
        lcd_update_ctrl_buff[LCD_UPDATE_SEQUENCE_LINE_BYTE_POSITION] += line;
        i2ct_lcd_update.status = 0;
        i2ct_lcd_update.address = SSD1306_I2C_ADDRESS;
        i2ct_lcd_update.len = sizeof(lcd_update_sequence);
        i2ct_lcd_update.data = lcd_update_ctrl_buff;
        i2ct_lcd_update.complete_callback = lcdUpdateDataCallback;
        i2cMasterTransaction(&i2ct_lcd_update, 0);
        // next
        next_update += (SAMPLE_RATE / LCD_FPS / 8); // 8 lines
        return true;
    }
    return false;
}
