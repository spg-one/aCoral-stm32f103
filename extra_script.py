Import("env")


env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJDUMP", "-S", "-d",  
        "$BUILD_DIR/${PROGNAME}.elf", ">", "$BUILD_DIR/${PROGNAME}.d"
    ]), "Building $BUILD_DIR/${PROGNAME}.d")
)