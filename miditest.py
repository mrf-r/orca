import numpy as np

def generate_addr(syx_size: int) -> bytes:
    "output may be less size than requested"
    if syx_size <= 128:
        length = syx_size
        a = np.arange(length)
        return bytes(a.astype(np.int8))
    elif (syx_size+1)/2 <= 128*128:
        length = int(syx_size/2)
        a = np.arange(length)
        a = ((a<<1)&0x7F00) | (a&0x7F)
        return bytes(a.astype(np.int16)) + bytes(syx_size-length*2)
    elif (syx_size+2)/3 <= 128*128*128:
        print(syx_size)
        length = int(syx_size/3)
        print(length*3)
        a = np.arange(length)
        a = ((a<<2)&0x7F0000) | ((a<<1)&0x7F00) | (a&0x7F)
        a = a.view(np.int8)
        p = np.arange(len(a))
        a = a[p%4!=3]
        return bytes(a) + bytes(syx_size-length*3)
    else: return bytes(0)

def generate_addrinv(syx_size: int) -> bytes:
    "output may be less size than requested"
    if syx_size <= 128:
        length = syx_size
        a = np.arange(length)
        a = (~a) & 0x7F
        return bytes(a.astype(np.int8))
    elif (syx_size+1)/2 <= 128*128:
        length = int(syx_size/2)
        a = np.arange(length)
        a = ~a
        a = ((a<<1)&0x7F00) | (a&0x7F)
        return bytes(a.astype(np.int16)) + bytes(syx_size-length*2)
    elif (syx_size+2)/3 <= 128*128*128:
        length = int(syx_size/3)
        a = np.arange(length)
        a = ~a
        a = ((a<<2)&0x7F0000) | ((a<<1)&0x7F00) | (a&0x7F)
        a = a.view(np.int8)
        p = np.arange(len(a))
        a = a[p%4!=3]
        return bytes(a) + bytes(syx_size-length*3)
    else: return bytes(0)

def filesave(name: str, arr: bytes)-> None:
    with open(name,"wb") as f:
        w = 0
        w+=f.write(b'\xF0')
        w+=f.write(arr)
        w+=f.write(b'\xF7')
        print(f"{len(arr)} bytes generated, {w} bytes written, {int((w+2)/3)} messages minimum")

filesave("testw.syx",generate_addr(3376813))
filesave("testiw.syx",generate_addrinv(3376813))
    
