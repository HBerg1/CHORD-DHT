MPIRUN = mpirun --oversubscribe
CC = mpicc
CFLAGS = -Wall -Wextra

all: DHT test_node DHT3

DHT: DHT.c
	$(CC) $(CFLAGS) -o DHT DHT.c

run: DHT
	$(MPIRUN) -np 11 DHT

DHT3: DHT3.c
	$(CC) $(CFLAGS) -o DHT3 DHT3.c

run3: DHT3
	$(MPIRUN) -np 11 DHT3

test_node: test_node.c
	gcc -o test_node test_node.c

clean:
	rm -f DHT test_node DHT3 caca