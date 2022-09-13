

# NETWORK CLIENT简介

这个项目将会介绍如何使用 `tuyaos 3 network client` 相关接口，创建客户端与服务器的 `Socket` 连接。

* `Socket` 简介
  
`Socket` 也叫`套接字`是应用层与 `TCP/IP` 协议族通信的中间软件抽象层，更直白的话， `Socket` 就是一组接口。我们可以直接调用这些接口，实现设备之间的网络通信，无需关心数据的组织、如何发送接收等底层原理。

![tcpip socket.png](https://airtake-public-data-1254153901.cos.ap-shanghai.myqcloud.com/content-platform/hestia/1655777228a883c8890b3.png)

* `Socket` 的文件描述符

起初 `Socket` 在 `Linux` 上跑的时候， `Socket` 的文件描述符就是通讯双方各自建立的对象。因为 `Linux` 系统上一切皆文件， `Socket` 也不例外，它就是可读/可写/可控制/可关闭的文件描述符。现在 `Socket` 网络编程已经可以在 `RTOS` 下使用了，只要你有硬件支持，网络编程的大概用法也传承了下来。所以不管是 `RTOS` 还是 `Linux` `，Socket` 的文件描述符的作用就是内核为了高效管理所创建的索引，即每创建一个 `Socket` 就对应一个文件描述符。文件描述符通常是一个大于0的整数。



* 客户端和服务器
  
客户端（ `Client` ）是发送请求（ `request` ），服务器端（ `Service` ）是响应请求（ `response` ）。例如有一款涂鸦的设备，要接入涂鸦 `IOT` 平台。那么涂鸦设备就是一个客户端，要发送接入 `IOT` 平台的请求；`IOT` 平台就是一个服务器，相响应涂鸦设备的接入请求。

* `TCP` 和 `UDP`

`TCP` 和 `UDP` 两个协议都在运输层，它们都是用于数据传输的。但是两者有很大的区别。

![tcp and udp 12138.png](https://airtake-public-data-1254153901.cos.ap-shanghai.myqcloud.com/content-platform/hestia/1655789761a1208d8ba6c.png)

* 字符串 `IP` 和整数 `IP` 

我们一般情况下讲的 `IP` 地址是字符串的`点分十进制形式`，例如`“192.168.31.102”`。但是在 `Socket` 网络编程中的讲的地址通常都是整数形式的 `IP` 地址，所以需要进行地址转换，可以直接调用接口函数 `tal_net_str2addr` 进行转换。字符串 `IP` 地址 `“192.168.31.102”` 对应的整数地址为1713350848。

* 端口
  
网络层为主机之间提供逻辑通讯，而 `Socket` 创建的 `TCP/UDP` 连接是运输层提供的`端到端`的逻辑通讯。在一个系统中，会有很多进程，端口号的作用就是把接收到的网络数据转交给某个进程，每个进程都有自己的端口号。

如下图所示，假设我需要建立一个 `TCP` 连接，左边设备的某个进程要发送数据给右边设备的进程1，他的端口号是7777，所以我建立的 `TCP` 连接的时候，不仅需要右边设备的 `IP` 地址` “192.168.56.3”`，还要建立连接进程的端口号7777。

![PORttttt  12138.png](https://airtake-public-data-1254153901.cos.ap-shanghai.myqcloud.com/content-platform/hestia/165579436062ce9b580d3.png)


## 流程介绍
相关函数目录：
`software/TuyaOS/include/components/tal_network/include/tal_network.h`

![cilent process 12138.png](https://airtake-public-data-1254153901.cos.ap-shanghai.myqcloud.com/content-platform/hestia/16557958665091e4b489c.png)

## 编译代码
到 `/TuyaOS` 目录下调用编译脚本，输入要编译的项目名称和版本号。
```shell
 ./build_app.sh apps/network_client network_client 1.0.0 
```


## 烧写
具体的烧录流程可参考此文档[烧录授权](https://developer.tuya.com/cn/docs/iot/flash-and-authorize?id=Kaytfatwdbfj2#title-7-%E4%BD%BF%E7%94%A8%E5%8E%9F%E5%8E%82%E5%B7%A5%E5%85%B7%E7%83%A7%E5%BD%95)。

## 运行结果
首先打开电脑热点。

随后使用 `Ubuntu` 虚拟机或者树莓派，只要是连着电脑网络的 `Linux` 系统即可。复制 `ubuntu_network_demo/server` 文件下的服务器源代码或者可执行文件，创建服务器。

运行可执行文件，创建一个服务器。
```sheel
./ubuntu_server 
```
服务器创建成功等待连接
```c
server:waiting for connections...
```

接着让客户端设备通过电脑热点接入 `IOT` 平台，此举能够确保客户端成功接入网络。然后设备客户端会自己执行代码建立连接。
如果一切正常，此时在服务器端就会提示连接成功。

```c
server:got connection from 192.168.137.95
```
服务器与客户端连接成功之后，客户端会发送 `“Hello,world!”` ，服务器接收到数据之后会返回`“Hello,world!”`。

如果客户端成功接收到服务器端返回的`“Hello,world!”`，那么关闭连接。

```c
[06-07 15:38:55 TUYA D][lr:0x4bb8b] client receive data:Hello,world!
[06-07 15:38:55 TUYA D][lr:0x4bb9d] close tcp socket!
```

如果不能建立 `Socket` 连接，有可能是电脑连接的网络有问题，不妨关闭电脑防火墙和电脑连接手机热点试试。
## 技术支持
您可以通过以下方法获得涂鸦的支持:
* [开发者中心](https://developer.tuya.com)
* [帮助中心](https://support.tuya.com/help)
* [技术支持帮助中心](https://service.console.tuya.com)
* [Tuya os](https://developer.tuya.com/cn/tuyaos)
