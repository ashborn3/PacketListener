build:
	gcc -o main main.c sqlite/sqlite3.c
run: build
	./main
clean:
	rm main