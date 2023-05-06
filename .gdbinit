# disable gdb warning that i don't understand)
set pagination off
set print pretty on
set output-radix 16

# load registers viewer
source gdb-svd.py
svd NUC123AE_v1.SVD
#svd NUC100_Series.SVD

# source obj/.gdbremote
target extended-remote :3333

# TODO: lma instead of __Vectors!!!!
define reload
    # monitor rtt stop
    monitor reset halt
    file obj/ORCA_FW.elf
    #file obj/ORCA_BOOT.elf
    load
    set $sp=__StackTop
    tb main
    continue
    # monitor rtt setup 0x20000000 65536 "SEGGER RTT"
    # source obj/.gdbrtt
    # monitor rtt start
    end

# reload

