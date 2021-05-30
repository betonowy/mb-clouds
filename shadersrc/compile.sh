#!/bin/bash

# compile all shaders into SPIR-V

which glslc >/dev/null

RET=$?

if [[ "$RET" -eq 1 ]]; then
  echo "Could not find glslc executable"
  exit 1
fi

SRC="shadersrc"
DST="res/shaders"

glslc "$SRC/test/shader.vert" -o "$DST/shader.vert.spv"
glslc "$SRC/test/shader.frag" -o "$DST/shader.frag.spv"
