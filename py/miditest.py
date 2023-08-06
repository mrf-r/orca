
# pip3 install python-rtmidi
import rtmidi
import time

# NAME = "Orca"
NAME = "nTour"

print("out connect...", end='')
midiout = rtmidi.MidiOut()
pc = midiout.get_port_count()
for i in range(pc):
    if NAME in midiout.get_port_name(i):
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
    if NAME in midiin.get_port_name(i):
        midiin.open_port(i)
        print("ok")
        break
    else:
        if i == (pc - 1):
            print("unconnected - something is wrong!")
            exit

print("sending random messages")
# midiout.send_message([0x80,60,0])
# midiout.send_message([0x80,61,0])
# midiout.send_message([0x80,62,0])
# midiout.send_message([0x80,60,0])
# midiout.send_message([0x80,61,0])
# midiout.send_message([0x80,62,0])
# midiout.send_message([0x80,60,0])
# midiout.send_message([0x80,61,0])
# midiout.send_message([0x80,62,0])
# midiout.send_message([0x80,60,0])
# midiout.send_message([0x80,61,0])
# midiout.send_message([0x80,62,0])

# print("playing few notes")
# midiout.send_message([129,60,100])
# time.sleep(1)
# midiout.send_message([145,60,100])
# midiout.send_message([129,62,100])
# time.sleep(1)
# midiout.send_message([145,62,100])
# midiout.send_message([129,64,100])
# time.sleep(1)
# midiout.send_message([145,64,100])

time.sleep(0.1)
loop = True
print("waitiing for 20 messages")
counter = 0

while loop:
    mes = midiin.get_message()
    if mes != None:
        print(f"in: {mes}")
        counter += 1
        if counter == 20:
            loop = False
    # else:
    #     loop = False

print("closing...", end='')
midiout.close_port()
midiout.delete()
midiin.close_port()
midiin.delete()
print("ok")
