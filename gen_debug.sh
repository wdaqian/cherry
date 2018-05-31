#!/bin/bash

gn gen out/debug --args="is_debug=true is_clang=false use_sysroot=false use_custom_libcxx=false use_custom_libcxx=false use_cxx14=true"
