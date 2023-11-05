# to be called from gdb or as a postbuild step
LOADADDRESS="$(arm-none-eabi-objdump obj/ORCA_FW.elf -x | grep __Vectors | awk -F" " '{print $1;}')"
RTTADDRESS="$(arm-none-eabi-objdump obj/ORCA_FW.elf -x | grep _SEGGER_RTT | awk -F" " '{print $1;}')"
RTTSIZE="$(arm-none-eabi-objdump obj/ORCA_FW.elf -x | grep _SEGGER_RTT | awk -F" " '{print $5;}')"
echo monitor rtt setup 0x$RTTADDRESS 0x$RTTSIZE '"SEGGER RTT"' > obj/.gdbrtt
