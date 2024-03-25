#ifndef _MIDI_CONF_H
#define _MIDI_CONF_H

// TEMPLATE

#include <orca.h>
#define MIDI_ASSERT ASSERT

typedef enum {
    MIDI_CN_LOCALPANEL = 0,
    MIDI_CN_USB,
} MidiCnEn;

extern volatile uint32_t counter_sr;

#define MIDI_GET_CLOCK() (counter_sr)
#define MIDI_CLOCK_RATE (SAMPLE_RATE)

extern volatile unsigned atomic_level;
static inline void _atomicStart()
{
    MIDI_ASSERT(atomic_level < 2);
    // TODO: should we get current irq state ?
    __disable_irq();
    atomic_level++;
}
static inline void _atomicEnd()
{
    MIDI_ASSERT(atomic_level);
    atomic_level--;
    if (0 == atomic_level) {
        __enable_irq();
    }
}

#define MIDI_ATOMIC_START _atomicStart
#define MIDI_ATOMIC_END _atomicEnd

#endif // _MIDI_CONF_H
