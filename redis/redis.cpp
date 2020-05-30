/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-29 21:55:46
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-29 22:45:03
 */

#include <hiredis/hiredis.h>

#include <iostream>

int main() {
  redisContext *context = redisConnect("localhost", 6379);
  /* { */
    std::cout << "errno = " << context->err << " errstr = " << context->errstr
              << std::endl;
  /*   redisReply *reply = */
  /*       static_cast<redisReply *>(redisCommand(context, "set name wms")); */

  /*   std::cout << "reply type = " << reply->type */
  /*             << " reply interger = " << reply->integer */
  /*             << " reply str = " << reply->str << std::endl; */

  /*   freeReplyObject(reply); */
  /* } */

  /* { */
  /*   redisReply *reply = nullptr; */
  /*   redisAppendCommand(context, "set key1 value"); */
  /*   redisAppendCommand(context, "get key2"); */
  /*   redisGetReply(context, reinterpret_cast<void **>(&reply));  // reply for set */
  /*   std::cout << "reply type = " << reply->type */
  /*             << " reply interger = " << reply->integer */
  /*             << " reply str = " << reply->str << std::endl; */
  /*   freeReplyObject(reply); */
  /*   redisGetReply(context, reinterpret_cast<void **>(&reply));  // reply for get */
  /*   std::cout << "reply type = " << reply->type */
  /*             << " reply interger = " << reply->integer */
  /*             << " reply str = " << reply->str << std::endl; */
  /*   freeReplyObject(reply); */
  /* } */
  redisFree(context);
  return 0;
}
