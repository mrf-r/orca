IP="$(cat /etc/resolv.conf | grep nameserver | awk -F" " '{print $2;}')"
echo $IP
echo bindto $IP > bindto.cfg
echo target extended-remote $IP:3333 > .gdbremote

# get loadaddr
#arm-none-eabi-objdump obj/ORCA_FW.elf -x | grep __Vectors | awk -F" " '{print $1;}'

#cmd.exe /c start /min cmd.exe /c e:/__gcc/xpack-openocd-0.11.0-3/bin/openocd.exe -f openocd-orca.cfg
cmd.exe /c start /min cmd.exe /c e:/__gcc/xpack-openocd-0.11.0-3/bin/openocd.exe -f openocd-orca.cfg
# swo and rtt
# cmd.exe /c start e:/__gcc/putty/PUTTY.exe -telnet -P 3456 $IP
cmd.exe /c start e:/__gcc/putty/PUTTY.exe -telnet -P 5678 $IP
gdb-multiarch

#cat /etc/resolv.conf | grep nameserver | awk '{gsub("nameserver","bindto")}1' > bindto.cfg
#/mnt/e/__gcc/xpack-openocd-0.11.0-3/bin/openocd.exe -f openocd-orca.cfg
#xterm -geometry +100+100 -e "openocd -f openocd-stm32f4disco.cfg" &
#sleep 1
#xterm -geometry +600+100 -e "telnet localhost 3456" &
#xterm -geometry 160x40+100+450 -e "gdb-multiarch -q" &  
#x-terminal-emulator -hold -e "gdb-multiarch -q" &
