# Monita Service ver 1.10

Monita Service for Monita4 Realtime Monitoring.

## **Get Started NOW**

### **Prerequisites**
```bash
## QT 5.6++
$ sudo apt install        \
    libqt5sql5            \
    libqt5sql5-mysql      \
    libqt5websockets5     \
    libqt5websockets5-dev \
    libqt5network5        \
    libqt5core5a          \
    qt5-qmake             \
    qtbase5-dev           \
    qtdeclarative5-dev    \
    qt5-qmake             \
    qt5-default           \
    qttools5-dev-tools
```

### **Setup**

```bash
$ git clone https://github.com/zenaki/Monita-Service.git
$ cd Monita-Service
$ git checkout monita-service_ver_1.10
```

Git submodules/dependencies automatically pulls in hiredis and QttpServer
```bash
$ git submodule update --init
```

build hiredis
```bash
$ cd Monita-Service/3rdparty/hiredis/
$ make
$ sudo make install
$ sudo ln -s /usr/local/lib/libhiredis.so.0.13 /usr/lib/x86_64-linux-gnu/libhiredis.so.0.13
```

build QttpServer
```bash
$ cd Monita-Service/3rdparty/QttpServer/
$ git submodule update --init
$ qmake qttpserver.pro
$ make
```

Using QtCreator, open `Monita_Service.pro` and enjoy!
or
Using QMake:
```bash
$ qmake CONFIG+=debug Monita_Service.pro
$ make
$ sudo cp monita-service /usr/local/bin/ && sudo chown root:root /usr/local/bin/monita-service
$ sudo cp monita-service-service /etc/init.d/monita-service
$ sudo chown root:root /etc/init.d/monita-service
$ sudo update-rc.d monita-service defaults
```

### **START MONITA SERVICE**
```bash
$ sudo service monita-service start
```
