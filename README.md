#### Build on linux
#### Install CMake
```
sudo apt-get install cmake
```
Build
```
cd columndb
mkdir build
cd build
cmake ..
make
```

run:
```
./columndb --port 8080
```


run in Docker:

```
cd columndb
docker build -t testdb .
docker run -p 8080:8080 --init --rm  testdb:latest
```
