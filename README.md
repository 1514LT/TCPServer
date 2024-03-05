# TCPServer

## 解压poco-poco-1.10.0.zip

unzip poco-poco-1.10.0.zip

cd poco-poco-1.10.0

## 安装三方库

```
sudo apt-get install openssl libssl-dev
sudo apt-get -y update && sudo apt-get -y install git g++ make cmake libssl-dev
```

## 构建poco库

```
mkdir cmake-build
cd cmake-build
cmake ..
make
sudo make install
```

安装的动态库会被保存在/usr/lib/x86_64-linux-gnu目录中，头文件会被保存在/usr/include中