build:
	gcc -o main packetProcessor.c sqlite/sqlite3.c
run: build
	./main
clean:
	rm main go_main libpacketProcessor.a packetProcessor.o sqlite3.o
build_object:
	gcc -c -fPIC packetProcessor.c -o packetProcessor.o
	gcc -c -fPIC sqlite/sqlite3.c -o sqlite3.o
	ar rcs libpacketProcessor.a packetProcessor.o sqlite3.o
build_go: build_object
	go build -o go_main main.go
run_go:
	./go_main
clean_go:
	rm go_main libpacketProcessor.a packetProcessor.o sqlite3.o
clean_db:
	rm packet_log.*