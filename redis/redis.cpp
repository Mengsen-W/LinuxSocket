/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-01 21:40:35
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-01 22:23:51
 */

#include <hiredis/hiredis.h>

#include <iostream>

int main() {
  redisContext *context = redisConnect("localhost", 6379);
  redisReply *reply = nullptr;

  reply = static_cast<redisReply *>(redisCommand(context, "flushall"));
  std::cout << "replay_type = " << reply->type
            << " reply interger = " << reply->integer
            << " reply len =  " << reply->len << " reply str = " << reply->str
            << " reply size =  " << reply->element << std::endl;
  freeReplyObject(reply);

  reply =
      static_cast<redisReply *>(redisCommand(context, "lpush mylist w m s"));

  std::cout << "replay_type = " << reply->type
            << " reply interger = " << reply->integer << std::endl;
  freeReplyObject(reply);

  reply =
      static_cast<redisReply *>(redisCommand(context, "lrange mylist 0 -1"));
  std::cout << "replay_type = " << reply->type
            << " reply size = " << reply->elements << std::endl;
  for (int i = 0; i < reply->elements; ++i) {
    std::cout << ((reply->element)[i])->str << std::endl;
  }
  freeReplyObject(reply);

  redisFree(context);
  return 0;
}
