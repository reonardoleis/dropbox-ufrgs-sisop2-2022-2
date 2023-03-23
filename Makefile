SERVER_FILES := $(shell find ./server ./commons -type f -iregex ".*\.cpp")
CLIENT_FILES := $(shell find ./client ./commons -type f -iregex ".*\.cpp")

build-server:
	g++ -g $(SERVER_FILES) -o ./server/bin/server2 -lpthread -std=c++0x

run-server:
	./server/bin/server

test-server: build-server run-server

build-client:
	g++ -g $(CLIENT_FILES) -o ./client/bin/client -lpthread -std=c++0x

run-client:
	./client/bin/client

test-client: build-client run-client

build:
	make build-server
	make build-client
