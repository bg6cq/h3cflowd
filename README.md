## h3cflowd: collect h3c router/firewall nat userlog(flowlog)

收集H3C路由器/防火墙输出的nat流日志(flowlog/userlog)。

参考文档：https://www.h3c.com/cn/d_201904/1173810_30005_0.htm

## 说明：

H3C路由器/防火墙可以将NAT流日志输出，本程序用来收集日志并存放为文件。

## 使用步骤：

1.  路由器/防火墙上增加配置：
```
 nat log enable
 nat log flow-begin
 nat log flow-end

 userlog flow export version 3
 userlog flow export source-ip 172.16.0.1
 userlog flow export host 172.16.21.2 port 4000
```

2. 建立目录
```
mkdir /natlog
```

3. 执行程序
```
./h3cflowd
```
程序在UDP 4000端口接收数据。

每收到100条日志打印1个"."字符。

日志存放在/natlog目录下，每天自动生成一个文件。

文件是.gz压缩的文本文件。

典型的内容如下：

8表示新建连接的记录、其他为删除连接的记录。删除连接时，后有该连接的持续时间。
```
23:00:17 tcp 8 172.17.1.83(202.3.6.7):39806(26169)->43.7.24.18:80
23:00:19 tcp 1 172.17.1.83(202.3.6.7):39806(26169)->43.7.24.18:80 TIME:3
```
