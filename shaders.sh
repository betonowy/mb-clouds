#!/bin/bash

# compile all shaders into SPIR-V
# requires glslc - A command-line GLSL/HLSL to SPIR-V compiler with Clang-compatible arguments.

which glslc >/dev/null

RET=$?

if [[ "$RET" -eq 1 ]]; then
  echo "Could not find glslc executable"
  exit 1
fi

SRC="shadersrc"
DST="res/shaders"

glslc -O "$SRC/test/shader.vert" -o "$DST/shader.vert.spv"
glslc -O "$SRC/test/shader.frag" -o "$DST/shader.frag.spv"
