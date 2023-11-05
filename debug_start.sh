# if variable not exist
if [ -v $WSL_DISTRO_NAME ] ; then
        echo "Linux localhost"
        echo bindto localhost > bindto.cfg
        echo target extended-remote localhost:3333 > .gdbremote
        openocd -f openocd-orca.cfg
        xterm -geometry +100+100 -e "openocd -f openocd-stm32f4disco.cfg" &
        sleep 1
        xterm -geometry +600+100 -e "telnet localhost 3456" &
        xterm -geometry 160x40+100+450 -e "gdb-multiarch -q" &  
        #x-terminal-emulator -hold -e "gdb-multiarch -q" &
else
        DIR_BIN=c:/portable/framework/bin
        echo "WSL mode"
        IP="$(cat /etc/resolv.conf | grep nameserver | awk -F" " '{print $2;}')"
        # cat /etc/resolv.conf | grep nameserver | awk '{gsub("nameserver","bindto")}1' > bindto.cfg
        echo $IP
        echo bindto $IP > bindto.cfg
        echo target extended-remote $IP:3333 > .gdbremote
        (cmd.exe /c start /min cmd.exe /c $DIR_BIN/openocd.exe -f openocd-orca.cfg &)
        (cmd.exe /c start $DIR_BIN/putty.exe -telnet -P 5678 $IP &)
        gdb-multiarch
fi
