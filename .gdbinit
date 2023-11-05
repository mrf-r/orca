# disable gdb warning that i don't understand)
set pagination off
set print pretty on
set output-radix 16

# load registers viewer
source gdb-svd.py
svd NUC123AE_v1.SVD
#svd NUC100_Series.SVD

# connect to openocd server gdb port
source .gdbremote

# TODO: lma instead of __Vectors!!!!
define reload
    monitor rtt stop
    monitor reset halt
    file obj/ORCA_FW.elf
    load
    set $sp=__StackTop
    tb main
    continue
    shell ./debug_rtt_update.sh
    source obj/.gdbrtt
    # monitor rtt setup 0x20000000 65536 "SEGGER RTT"
    monitor rtt start
    end

define rebuild
    shell make firmware DEBUG=1 RAMLINK=1 -j
    reload
    end

reload

