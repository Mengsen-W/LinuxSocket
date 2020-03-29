server : chat_Server.cpp Protocal.pb.cc Protocal.pb.h chat_message.h structHeader.h JSONObj.h serialization.h
	g++ -Og -std=c++17 -fsanitize=address -fno-omit-frame-pointer -Wall -g -o chat_server.out Protocal.pb.cc chat_Server.cpp -lboost_system -lboost_serialization -lpthread -lprotobuf
client : chat_Client.cpp Protocal.pb.cc Protocal.pb.h chat_message.h structHeader.h JSONObj.h serialization.h
	g++ -Og -std=c++17 -fsanitize=address -fno-omit-frame-pointer -Wall -g -o chat_client.out chat_Client.cpp Protocal.pb.cc -lboost_system -lboost_serialization -lpthread -lprotobuf
