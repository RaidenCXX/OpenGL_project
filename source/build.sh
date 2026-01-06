
#!/usr/bin/env bash
set -e

cd "$(dirname "$0")/../build"
cmake --fresh ..
make
./lernOpenGL
