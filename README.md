# SocketExperiment
2020吉林大学计算机网络实验课
***
1. 协议分析
    * SocketExperiment.cpp
    * header.h
2. FTP并发
***
实验指导书上偏移计算有误，实际情况需要百度TCP、IP、UDP的头部
根据头部各个字段的长度通过位运算得到值，某些特殊字段是网络流
的形式，需要ntoh*()函数大家族进行转换，然后如果想实现wireshark
那样的形式需要在指导书的基础上添加WSAIoctl(将套接字设置为接受
所有发出和收入的包)、ioctlsocket(设置为阻塞)，这两块我也不是很
懂，配置方法参考了<https://www.cnblogs.com/alexhg/p/6531527.html>
***
### 三次握手
![握手1](https://github.com/zyc573823770/SocketExperiment/blob/master/%E6%8F%A1%E6%89%8B1.png)  
![握手2](https://github.com/zyc573823770/SocketExperiment/blob/master/%E6%8F%A1%E6%89%8B2.png)  
![握手3](https://github.com/zyc573823770/SocketExperiment/blob/master/%E6%8F%A1%E6%89%8B3.png)  
### 发送消息
![消息](https://github.com/zyc573823770/SocketExperiment/blob/master/%E6%B6%88%E6%81%AF%E5%8F%91%E9%80%81.png)  
### 四次挥手
![挥手1](https://github.com/zyc573823770/SocketExperiment/blob/master/%E6%8C%A5%E6%89%8B1.png)  
![挥手2](https://github.com/zyc573823770/SocketExperiment/blob/master/%E6%8C%A5%E6%89%8B2.png)  
### 公网（这个能看出来是个POST其他的看不太懂）
![公网](https://github.com/zyc573823770/SocketExperiment/blob/master/%E5%85%AC%E7%BD%91.png)  
(PS：图片看不了的话，把相关dns加到本地dns中，百度一下，你也不一定会)
