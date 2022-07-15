# this description is from another project

 - install build-essential / cygwin (for make)
 - install gcc-arm-none-eabi
 - `make debug GCC_PATH=C:/gnu/gcc10-2020-q4-major/bin -j12 --output-sync`
 - add `git` to path

# git versioning

 - use `git flow`
 - when release, use tags "%d.%d.%d" with annotations, example: `git flow release start 1.1.3`

# Debug

 - gdb with python support (if you want low level registers view)
 - `pip install -U ` `cmsis-svd` `terminaltables`. https://github.com/1udo6arre/svd-tools
 - openOCD 0.11.0-1 differs from 0.11.0-3 . uncomment apropriate lines in `openocd-stm32f4disco.cfg`
 - run `start_debug.sh` from `bootloader` folder

# Other info

asm output `arm-none-eabi-objdump -S any.o`
