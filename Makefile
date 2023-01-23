SERVER_FILES := $(shell find ./server ./commons -type f -iregex ".*\.cpp")
CLIENT_FILES := $(shell find ./client ./commons -type f -iregex ".*\.cpp")

build-server:
	g++ $(SERVER_FILES) -o ./server/bin/server

run-server:
	./server/bin/server

test-server: build-server run-server

build-client:
	g++ $(CLIENT_FILES) -o ./client/bin/client

run-client:
	./client/bin/client

test-client: build-client run-client
