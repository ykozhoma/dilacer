**Build and run instructions**
1. Set executable permissions for `build.sh` script and run it
```
chmod +x ./build.sh
./build.sh
```
2. Run the program
```
LD_LIBRARY_PATH=./jpeg-9e/out/lib ./dilace interlaced.jpg out.jpg
```