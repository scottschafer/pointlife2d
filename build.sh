emmake make
rm -f bin/release/PointLife.bc
mv bin/release/PointLife bin/release/PointLife.bc
emcc -O3 bin/release/PointLife.bc -o pl.html
