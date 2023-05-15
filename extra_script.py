Import("env")

def disassembly(source, target, env):
    env.Execute("C:/Users/SPGGO/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-objdump -S -d C:/Users/SPGGO/Documents/aCoral/aCoral-stm32f103/.pio/build/genericSTM32F103RC/firmware.elf > ./.pio/build/genericSTM32F103RC/firmware.d")

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJDUMP", "-S", "-d",  
        "$BUILD_DIR/${PROGNAME}.elf", ">", "$BUILD_DIR/${PROGNAME}.d"
    ]), "Building $BUILD_DIR/${PROGNAME}.d")
)