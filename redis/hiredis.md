# Hiredis API
---
## 1. 编译安装
1. git clone https://github.com/redis/hiredis.git
2. make
3. make install

## 2. 同步 API 接口
1. redisContext \*redisConnect(const char \*ip, int port);
     1. redisConnect函数用来创建一个redisContext，是一个指向本链接的结构体
     2. 它包含了连接相关的信息，里面有个err字段，0表示正常，其他表示出错，通过errstr字段可以知晓错误信息。
2. void \*redisCommand(redisContext \*c, const char \*format, ...);
     1. redisCommand的调用格式类似printf函数，输入二进制格式的value内容，其后必须表明二进制的字节长度。
     2. redisCommand函数返回redisReply，我们需要通过判断它的type字段来明确具体返回
3. void \*redisCommandArgv(redisContext \*c, int argc, const char \*\*argv, const size\_t \*argvlen);
     1. argv 存放每个命令参数的指针的字符串数组名
     2. argc 存放命令参数的个数
     3. argclen 存放每个参数的指定长度，而不是依靠\0来判断，存放int的数组名
4. void redisAppendCommand(redisContext \*c, const char \*format, ...);
5. void redisAppendCommandArgv(redisContext \*c, int argc, const char \*\*argv, const size\_t \*argvlen);
6. int redisGetReply(redisContext \*context,redisReply\*\* reply);
     1. 将管道内的缓存写入redis
     2. 并返回由redis返回的第一个 reply
7. void freeReplyObject(void \*reply);
