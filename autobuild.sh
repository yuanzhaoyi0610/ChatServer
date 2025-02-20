set -x
rm -rf "$PWD/build/*"
cd "$PWD/build" || exit 1
cmake ..
make

