#!/bin/bash
set -eu
cd "$(dirname "$0")"

# --- Unpack Arguments --------------------------------------------------------
for arg in "$@"; do declare $arg='1'; done
if [ -z "${gcc:+1}" ];       then clang=1; fi
if [ -z "${release:+1}" ];   then debug=1; fi
if [ -n "${debug:+1}" ];     then echo "[debug mode]"; fi
if [ -n "${release:+1}" ];   then echo "[release mode]"; fi
if [ -n "${clang:+1}" ];     then compiler="${CC:-clang++}"; echo "[clang compile]"; fi
if [ -n "${gcc:+1}" ];       then compiler="${CC:-g++}"; echo "[gcc compile]"; fi

# --- Unpack Command Line Build Arguments -------------------------------------
auto_compile_flags=''

# --- Compile/Link Line Definitions -------------------------------------------
clang_common="-I../src/ -g"
clang_common="$clang_common -Wno-unknown-warning-option -fdiagnostics-absolute-paths -Wall -Wno-missing-braces -Wno-unused-function -Wno-writable-strings -Wno-unused-value -Wno-unused-variable -Wno-unused-local-typedef -Wno-deprecated-register -Wno-deprecated-declarations -Wno-unused-but-set-variable -Wno-single-bit-bitfield-constant-conversion -Wno-compare-distinct-pointer-types -Wno-initializer-overrides -Wno-incompatible-pointer-types-discards-qualifiers -Wno-for-loop-analysis -Xclang -flto-visibility-public-std -D_USE_MATH_DEFINES -Dstrdup=_strdup -Dgnu_printf=printf"
clang_common="$clang_common -std=c++17"
clang_common="$clang_common -framework OpenGL -I$(realpath third_party/raylib-master/src/) -L$(realpath third_party/raylib-master/src/) -lraylib"
clang_common="$clang_common -framework Cocoa -framework IOKit -framework CoreVideo"
clang_common="$clang_common -I$(realpath third_party/raygui/src/)"
clang_debug="$compiler -g -O0 -DBUILD_DEBUG=1 ${clang_common} ${auto_compile_flags}"
clang_release="$compiler -g -O2 -DBUILD_DEBUG=0 ${clang_common} ${auto_compile_flags}"
clang_link=""
clang_out="-o"
gcc_common='-I../src/ -g -Wno-unknown-warning-option -Wall -Wno-missing-braces -Wno-unused-function -Wno-attributes -Wno-unused-value -Wno-unused-variable -Wno-unused-local-typedef -Wno-deprecated-declarations -Wno-unused-but-set-variable -Wno-compare-distinct-pointer-types -D_USE_MATH_DEFINES -Dstrdup=_strdup -Dgnu_printf=printf'
gcc_debug="$compiler -g -O0 -DBUILD_DEBUG=1 ${gcc_common} ${auto_compile_flags}"
gcc_release="$compiler -g -O2 -DBUILD_DEBUG=0 ${gcc_common} ${auto_compile_flags}"
# gcc_link="-lpthread -lm -lrt -ldl"
gcc_link=""
gcc_out="-o"

# --- Per-Build Settings ------------------------------------------------------
link_dll="-fPIC"
# link_os_gfx="-lX11 -lXext"

# --- Choose Compile/Link Lines -----------------------------------------------
if [ -n "${gcc:+1}" ];     then compile_debug="$gcc_debug"; fi
if [ -n "${gcc:+1}" ];     then compile_release="$gcc_release"; fi
if [ -n "${gcc:+1}" ];     then compile_link="$gcc_link"; fi
if [ -n "${gcc:+1}" ];     then out="$gcc_out"; fi
if [ -n "${clang:+1}" ];   then compile_debug="$clang_debug"; fi
if [ -n "${clang:+1}" ];   then compile_release="$clang_release"; fi
if [ -n "${clang:+1}" ];   then compile_link="$clang_link"; fi
if [ -n "${clang:+1}" ];   then out="$clang_out"; fi
if [ -n "${debug:+1}" ];   then compile="$compile_debug"; fi
if [ -n "${release:+1}" ]; then compile="$compile_release"; fi

# --- Prep Directories --------------------------------------------------------
mkdir -p build

# --- Build Everything (@build_targets) ---------------------------------------
cd build
if [ -n "${main:+1}" ];                then didbuild=1 && $compile ../src/main.cpp                                    $compile_link $out main; fi
cd ..

# --- Warn On No Builds -------------------------------------------------------
if [ -z "${didbuild:+1}" ]
then
  echo "[WARNING] no valid build target specified; must use build target names as arguments to this script, like \`./build.sh main\`"
  exit 1
fi
