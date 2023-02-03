main: p1_678.o
	cc -o main p1_678.o

p1_678.o: p1_678.c
	cc -c p1_678.c

clean:
	rm main p1_678.o