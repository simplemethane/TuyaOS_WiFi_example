

# NETWORK SERVER

##  简介

这个项目将会介绍如何使用 `tuyaos 3 network server` 相关接口，创建一个服务端，等待客户端连接之后关闭服务器。

Socket 的其他相关知识在 `network_client` 的例程下已有介绍。

* `Socket` 服务器特有函数
  
接下来会介绍作为服务器特有的3个重要函数。

![server bind listen accept 12138 .png](https://airtake-public-data-1254153901.cos.ap-shanghai.myqcloud.com/content-platform/hestia/1655798571e2dfff47b21.png)

* socket 通信流程

假设建立一个 TCP 连接，客户端和服务器的通信流程如下图所示：

![service and cilent process .png](https://airtake-public-data-1254153901.cos.ap-shanghai.myqcloud.com/content-platform/hestia/1655799922a9c0c801cc3.png)




## 流程介绍
相关函数目录：
`software/TuyaOS/include/components/tal_network/include/tal_network.h`

![server process .png](https://airtake-public-data-1254153901.cos.ap-shanghai.myqcloud.com/content-platform/hestia/1655800225b04514f051e.png)

## 编译代码
到 `/TuyaOS` 目录下调用编译脚本，输入要编译的项目名称和版本号。
```shell
 ./build_app.sh apps/network_server network_server 1.0.0 
```


## 烧写
具体的烧录流程可参考此文档[烧录授权](https://developer.tuya.com/cn/docs/iot/flash-and-authorize?id=Kaytfatwdbfj2#title-7-%E4%BD%BF%E7%94%A8%E5%8E%9F%E5%8E%82%E5%B7%A5%E5%85%B7%E7%83%A7%E5%BD%95)。

## 运行结果
首先打开电脑热点。

接着让服务器设备通过电脑热点接入 `IOT` 平台，此举能够确保服务器设备成功接入网络。

然后服务器设备会自己创建服务器，等待客户端连接。

随后使用 `Ubuntu` 虚拟机或者树莓派，只要是连着电脑网络的 `Linux` 系统即可。复制 `ubuntu_network_demo/cilent` 文件下的客户端源代码或者可执行文件，创建客户端去连接服务器。

运行客户端可执行文件 , `192.168.137.95` 为服务器设备的 `IP` 地址
```sheel
./cilent 192.168.137.95
```
客户端成功连接到服务器。
```C
client:connecting to 192.168.137.95
```

服务器接收到了客户端的连接请求，打印客户端的 `IP` 地址和端口号。随后关闭 `Socket` 连接。
```c
[06-07 15:38:55 TUYA D][lr:0x4bb61] connection from:192.168.137.1, port:51016
```


## 技术支持
您可以通过以下方法获得涂鸦的支持:
* [开发者中心](https://developer.tuya.com)
* [帮助中心](https://support.tuya.com/help)
* [技术支持帮助中心](https://service.console.tuya.com)
* [Tuya os](https://developer.tuya.com/cn/tuyaos)
