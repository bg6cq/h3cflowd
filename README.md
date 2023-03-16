# h3cflowd: collect h3c router nat userlog(flowlog)

收集H3C路由器输出的流日志(flowlog/userlog)。

参考文档：https://www.h3c.com/en/Support/Resource_Center/HK/Routers/H3C_SR6600-X_Series_Routers/H3C_SR6600-X_Series_Routers/Technical_Documents/Configure___Deploy/Configuration_Guides/H3C_SR6600_SR6600-X_CG-R7607-6W100/13/201704/990159_294551_0.htm

# 使用步骤：

1.  路由器上增加配置：
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

日志存放在/natlog目录下

