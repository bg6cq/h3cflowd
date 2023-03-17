## h3cflowd: collect h3c router/firewall nat userlog(flowlog)

## 说明：

收集H3C路由器/防火墙输出的nat流日志(flowlog/userlog)。

H3C路由器/防火墙可以将NAT流日志输出，本程序用来收集日志并存放为文件。

参考文档：https://www.h3c.com/cn/d_201904/1173810_30005_0.htm

## 使用步骤：

1. H3C的路由器/防火墙上增加配置，输出NAT流日志：
```
 nat log enable     ; 启用nat流日志
 nat log flow-begin ; 记录流开始信息，会产生更多日志，如果节省空间可以不启用
 nat log flow-end   ; 记录流结束信息，必须启用

 userlog flow export version 3
 userlog flow export source-ip 172.16.0.1       ; 流日志的源IP，设备的某个IP
 userlog flow export host 172.16.21.2 port 4000 ; 日志收集服务器，172.16.21.2 是运行本程序的机器
```

2. 建立目录，用来存放日志
```
mkdir /natlog
```
日志存放在/natlog目录下，每天自动生成一个文件。

如果 / 目录下空间少，/home 目录下空间多，可以建立目录 /home/natlog  并生成 /natlog 软链接
```
mkdir /home/natlog
ln -s /home/natlog /natlog
```

3. 下载、编译程序

首先应该安装gcc、git软件。
```
cd /usr/src
git clone https://github.com/bg6cq/h3cflowd.git
cd h3cflowd
make
```

4. 测试执行程序
```
./h3cflowd -d
```
程序在UDP 4000端口接收数据，因此要修改防火墙规则，允许接收UDP 4000端口数据包。

如果有日志输出说明工作正常。

5. 执行程序

请参考 run.sh 设置开机启动执行程序。

run.sh可以在系统启动时执行，比如在 /etc/rc.d/rc.local 中增加
```
screen -d -m /usr/src/h3cflowd/run.sh  &
```

每收到100条日志打印1个"."字符。

日志存放在/natlog目录下，是.gz压缩的文本文件，典型的内容如下：

```
23:00:17 tcp 8 172.17.1.83(202.3.6.7):39806(26169)->43.7.24.18:80
23:00:19 tcp 1 172.17.1.83(202.3.6.7):39806(26169)->43.7.24.18:80 TIME:3
```
8表示新建连接的记录、其他为删除连接的记录。删除连接的记录，后有该连接的持续时间。

6. 日志文件清理

请参考 crontab.txt，每天清理超过200天的日志文件。

