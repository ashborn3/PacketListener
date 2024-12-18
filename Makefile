build:
	gcc -o main main.c
run: build
	./main
clean:
	rm main