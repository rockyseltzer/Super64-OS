# Source this before running make. N64_INST is the libdragon toolchain prefix
# (libdragon b26fce6 with its prebuilt MIPS64 GCC, plus libmad).
export N64_INST="${N64_INST:-/opt/libdragon}"
export PATH="$N64_INST/bin:$PATH"
