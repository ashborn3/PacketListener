build:
	gcc -c -fPIC packetProcessor.c -o packetProcessor.o
	gcc -c -fPIC sqlite/sqlite3.c -o sqlite3.o
	ar rcs libpacketProcessor.a packetProcessor.o sqlite3.o
	go build -o go_main main.go
run:
	./go_main
clean:
	rm main go_main libpacketProcessor.a packetProcessor.o sqlite3.o
clean_go:
	rm go_main libpacketProcessor.a packetProcessor.o sqlite3.o
clean_db:
	rm packet_log.*