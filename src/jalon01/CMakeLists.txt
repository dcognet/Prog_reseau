cmake_minimum_required(VERSION 2.8.3)

project(RE216_JALON01)

#add here source files you need to compile and link to the client app
SET(SRC_CLIENT
	include/client_tools.h
	include/client_tools.c
	client.c
	)


add_executable(RE216_JALON01_CLIENT ${SRC_CLIENT})


#add here source files you need to compile and link to the server app
SET(SRC_SERVER
				include/server_tools.h
				include/server_tools.c
				include/user_tools.h
				include/user_tools.c
				include/server_cast.h
				include/server_cast.c
				include/server_channel.h
				include/server_channel.c
        server.c)


add_executable(RE216_JALON01_SERVER  ${SRC_SERVER})
