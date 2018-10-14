cmake_minimum_required(VERSION 2.8.3)

project(RE216_JALON01)

#add here source files you need to compile and link to the client app
SET(SRC_CLIENT
	client_tools.h
	client_tools.c
	client.c
	)


add_executable(RE216_JALON01_CLIENT ${SRC_CLIENT})


#add here source files you need to compile and link to the server app
SET(SRC_SERVER
				server_tools.h
				server_tools.c
				user_tools.h
				user_tools.c
				server_cast.h
				server_cast.c
        server.c)


add_executable(RE216_JALON01_SERVER  ${SRC_SERVER})
