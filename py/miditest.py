
import rtmidi
import time

print("out connect...", end='')
midiout = rtmidi.MidiOut()
pc = midiout.get_port_count()
for i in range(pc):
    if "Orca" in midiout.get_port_name(i):
        midiout.open_port(i)
        print("ok")
        break
    else:
        if i == (pc - 1):
            print("unconnected")
            exit

print("in connect...", end='')
midiin = rtmidi.MidiIn()
pc = midiin.get_port_count()
for i in range(pc):
    if "Orca" in midiin.get_port_name(i):
        midiin.open_port(i)
        print("ok")
        break
    else:
        if i == (pc - 1):
            print("unconnected - something is wrong!")
            exit

print("test:")
midiout.send_message([0x80,60,0])
midiout.send_message([0x80,61,0])
midiout.send_message([0x80,62,0])

time.sleep(0.1)
loop = True
while loop:
    mes = midiin.get_message()
    if mes != None:
        print(f"in: {mes}")
    else:
        loop = False

print("closing...", end='')
midiout.close_port()
midiout.delete()
midiin.close_port()
midiin.delete()
print("ok")
