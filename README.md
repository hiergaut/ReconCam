# ReconCam
Auto movements recognition with opencv


# Build and execute on PC

make CFLAGS="-D PC"

echo 1 > gpio/gpio10/value #no gpio on pc

./a.out --sensor=10


# Build and execute on Raspberry pi
make

./a.out --sensor=10 #num gpio of IR sensor on pi

## Capture movement
./a.out --sensor=10 #num gpio of IR sensor on pi
[![](https://img.youtube.com/vi/CuX1CJ8I_7Y/0.jpg)](https://youtu.be/CuX1CJ8I_7Y "view on youtube")

## Learning for future movement recognition
On root repository
```bash
mkdir build
cd build
qmake ../LearningUI
make 
```
### Running
./LearningUI
[![](https://img.youtube.com/vi/AcWnUihFzlE/0.jpg)](https://youtu.be/AcWnUihFzlE "view on youtube")
