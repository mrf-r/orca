# disable gdb warning that i don't understand)
set pagination off
set print pretty on

# load registers viewer
source gdb-svd.py
svd NUC123AE_v1.SVD
#svd NUC100_Series.SVD

source .gdbremote
#file obj/ORCA_FW.elf

# define runto
#     b $arg0
#     continue
#     clear $arg0
#     end

# TODO: lma instead of __Vectors!!!!
define reload
    monitor reset halt
    #file obj/ORCA_FW.elf
    file obj/ORCA_BOOT.elf
    load
    set $sp=__StackTop
    tb main
    continue
    monitor rtt setup 0x20000000 65536 "SEGGER RTT"
    monitor rtt start
    end

reload

