## h3cflowd: collect H3C router/firewall NAT userlog(flowlog) Flow 3.0

## 说明：

收集H3C路由器/防火墙输出的NAT流日志(userlog/flowlog)。

H3C路由器/防火墙可以将NAT流日志输出，本程序用来收集日志(Flow 3.0)并压缩存放为文件。

参考文档：https://www.h3c.com/cn/d_201904/1173810_30005_0.htm

## 使用步骤：

### 1. H3C路由器/防火墙上增加配置，输出NAT流Flow 3.0日志：
```
 nat log enable     ; 启用NAT流日志
 nat log flow-begin ; 记录流开始信息，会产生更多日志，如果节省空间可以不启用
 nat log flow-end   ; 记录流结束信息，必须启用

 userlog flow export version 3
 userlog flow export source-ip 172.16.0.1       ; 流日志的源IP，设备的某个IP
 userlog flow export host 172.16.21.2 port 4000 ; 日志收集服务器，172.16.21.2 是运行本程序的机器

 session statistics enable                      ; 如果收集的日志中没有数据包/字节统计，可以启用统计
```

### 2. 建立目录，用来存放日志
```
mkdir /natlog
```
日志压缩存放在/natlog目录下，每天自动生成一个文件。日志文件大小与网络规模和用户使用习惯有关，500个IP左右的网络，每天日志约1-2GB。

如果 / 目录下空间少，/home 目录下空间多，可以建立目录 /home/natlog  并生成 /natlog 软链接
```
mkdir /home/natlog
ln -s /home/natlog /natlog
```

### 3. 下载、编译程序

首先安装gcc、git软件，然后
```
cd /usr/src
git clone https://github.com/bg6cq/h3cflowd.git
cd h3cflowd
make
```

### 4. 测试执行程序
```
./h3cflowd -d
```
程序在UDP 4000端口接收流日志数据，因此要修改防火墙规则，允许接收UDP 4000端口数据包。

如果有日志输出说明工作正常。

### 5. 执行程序

请参考 run.sh 设置开机启动执行程序。

run.sh 可以在系统启动时执行，比如在 /etc/rc.d/rc.local 中增加
```
screen -d -m /usr/src/h3cflowd/run.sh  &
```
并`chmod u+x /etc/rc.d/rc.local`。

程序正常执行时，每收到100个UDP流日志包显示"."字符，每显示30个"."字符会显示接收到的UDP包、流日志统计信息。
如果您的系统流日志速率高，建议用命令参数`-n num`指定更多包再显示"，"，如`-n 1000`指定接收1000个UDP包时显示"."字符。

注意：如果因程序更新需要停止已在运行的程序，建议使用`kill -HUP pid_of_h3cflowd`停止，这样可以将正在压缩的日志完整存储。否则直接kill进程最后的日志未完整保存。

### 5. 日志文件说明

日志存放在/natlog目录下，是.gz压缩的文本文件。日志文件累计到一定大小才会写磁盘，因此如果日志量少，请多等一些时间才会有文件。日志文件可以使用`zless /natlog/XXXX 或 zgrep YYYYY /natlog/XXXX`查看或检索。日志文件典型的内容如下：

```
08:21:24 tcp 8 172.16.16.41(5.40.12.10):64886(9104)->6.10.6.17:80 1/64 0/0
08:21:33 tcp 2 172.16.16.41(5.40.12.10):64886(9104)->6.10.6.17:80 770/33929 2213/3269740 TIME:9
```

时间戳来自于NAT流日志信息，由路由器/防火墙产生，建议路由器/防火墙使用ntp对时，确保时间的准确。

tcp/udp后的8/2是操作字，表示记录流日志的原因。

8表示是连接新建的记录。如果路由器/防火墙上不配置`nat log flow-begin`则无连接新建记录，可以节省存储空间。`1/64 0/0`是发送数据包/发送字节 接收数据包/接收字节。如果缺少这部分信息，请试着在路由器/防火墙上增加`session statistics enable`配置。

其他为连接结束的记录，`770/33929 2213/3269740`是发送数据包/发送字节 接收数据包/接收字节，TIME:9 是该连接的持续时间。

操作字对应的记录生成流日志的原因如下：
```
0：保留不用
1：正常流结束
2：定时器超时老化
3：清除配置/配置变动引起的流老化
4：资源不足带来的流老化
5：保留不用
6：活跃流定期记录其连接情况
7：新的流创建触发强制删除原有流
8：流创建
FE：其他
10~FE-1：以后扩充用
```

此外，本程序还支持输出json格式(命令行使用 `-j`选项)流日志文件，以方便使用fluentd、filebeat之类的软件读取并转发到ELK系统。
json格式文件是未压缩文本文件，占用空间较大，是默认压缩日志文件的20倍左右，因此使用json格式输出时，要关注及时清理文件。

### 7. 日志文件清理

请参考 crontab.txt，自动清理超过200天的日志文件。

### 8. 程序命令行

`./h3cflowd -h`输出如下帮助信息

```
  collect H3C router/firewall NAT userlog(flowlog)

  h3cflowd [ -h ] [ -d ] [ -p port ] [ -w work_dir ]
        -h            print help message
        -d            enable debug
        -p port       udp port, default is 4000
        -n number     number of udp packets to print ., default is 100
        -w work_dir   directory to save log file, default is /natlog
        -j            store log files in json format,
                      for fluentd or filebeat to read

 Note: send KILL signal cause h3cflowd to terminate gracefully.
```

### 9. 系统优化

对于Linux系统，建议修改以下参数以设置足够大的UDP缓冲区，减少丢包的可能。
```
sysctl -w net.core.rmem_max=4194304
```

### 10. 日志容量估算

每秒钟新建NAT连接的数量大致决定了日志的多少。

路由器/防火墙上执行`display session statistics`可以看到每秒钟新建的连接数(Session establishment rate)，从而可以估算出日志文件需要的空间。

下面是一个显示的例子，每秒钟新建连接是570。
```
Router>dis session statistics
Slot 0:
Current sessions: 28817
          TCP sessions:                13064
          UDP sessions:                15539
         ICMP sessions:                  213
        RAWIP sessions:                    1

Current relation-table entries: 1

Session establishment rate: 570/s
          TCP:                 208/s
          UDP:                 349/s
         ICMP:                  13/s
```

以每秒钟1000个新建连接估算，如果每个连接产生2条流日志，每天产生172.8M(1.72亿)条流日志。
原始Flow 3.0流日志UDP数据包为11GB，转换为文本后约为14GB，压缩后大约占用2.5GB空间。如果使用json格式文件，则文件大小约45GB。

此时路由器/防火墙每秒钟发给流日志采集记录服务器1000*2/15=133个UDP包，约136KB字节，占用带宽1.1Mbps。

这大约是1Gbps，500人左右上网的日志规模。

### 11. 其他

使用中有任何问题或建议，欢迎联系 james@ustc.edu.cn
