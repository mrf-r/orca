#include "mgl.h"
#include "orca.h"

// *****************************************
// !!! THIS IS LOCAL ORCA IMPLEMENTATION !!!
// *****************************************
// criticalloop added to the pixel output

uint8_t mgl_framebuffer[DISPLAY_SIZE_X * DISPLAY_SIZE_Y / 8];
static uint16_t fwaxs;
static uint16_t fways;
static uint16_t fwaxe;
static uint16_t fwaye;
static uint16_t fx;
static uint16_t fy;

__attribute__((weak)) void mglDispSetZone(const uint16_t wax, const uint16_t way, const uint16_t wax_size, const uint16_t way_size)
{
    fwaxs = wax, fways = way, fwaxe = wax + wax_size, fwaye = way + way_size;
    MGL_ASSERT(fwaxs < DISPLAY_SIZE_X);
    MGL_ASSERT(fwaxe <= DISPLAY_SIZE_X);
    MGL_ASSERT(fways < DISPLAY_SIZE_Y);
    MGL_ASSERT(fwaye <= DISPLAY_SIZE_Y);
    fx = fwaxs, fy = fways;
}

__attribute__((weak)) void mglDispPixelOut(MglColor c)
{
    if (c.wrd == COLOR_OFF.wrd)
        mgl_framebuffer[fy / 8 * DISPLAY_SIZE_X + fx] &= ~(1 << (fy & 0x7));
    else if (c.wrd == COLOR_ON.wrd)
        mgl_framebuffer[fy / 8 * DISPLAY_SIZE_X + fx] |= 1 << (fy & 0x7);
    else if (c.wrd == COLOR_INVERT.wrd)
        mgl_framebuffer[fy / 8 * DISPLAY_SIZE_X + fx] ^= 1 << (fy & 0x7);

    fx++;
    if (fx >= fwaxe) {
        fx = fwaxs;
        fy++;
        if (fy >= fwaye) {
            fy = fways;
        }
    }
    criticalLoop();
}

__attribute__((weak)) void mglDispUpdate() { }

#ifndef MGL_SINGLEDISPLAY
static MglDispContext context;
const MglDisplay mgl_display = {
    .context = &context,
    .size_x = DISPLAY_SIZE_X,
    .size_y = DISPLAY_SIZE_Y,
    .setZone = mglDispSetZone,
    .pixelOut = mglDispPixelOut,
    .update = mglDispUpdate
};
#endif
