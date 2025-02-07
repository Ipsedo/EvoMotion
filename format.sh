#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

find "${SCRIPT_DIR}/src/" -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
find "${SCRIPT_DIR}/evo_motion_model/" -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
find "${SCRIPT_DIR}/evo_motion_networks/" -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
find "${SCRIPT_DIR}/evo_motion_view/" -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i