build:
	gcc -o main main.c sqlite/sqlite3.c
run: build
	./main
clean:
	rm main
build_object:
	gcc -c -fPIC packetProcessor.c -o packetProcessor.o
	gcc -c -fPIC sqlite/sqlite3.c -o sqlite3.o
	ar rcs libpacketProcessor.a packetProcessor.o sqlite3.o
	rm packetProcessor.o sqlite3.o
build_go: 
	go build -o go_main main.go
run_go: build_go
	./go_main