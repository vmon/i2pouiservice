# How to build

```
mkdir build
cd build
cmake ..
make -j `nproc`
```

# How to debug 

```
cd src
gdb -i=mi --args ./get_slash 127.0.0.1 /
```

