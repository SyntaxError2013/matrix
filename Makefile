CC=gcc
CFLAGS= -Wall

default: server client

server: server.o
	@echo Compiling server
	$(CC) $(CFLAGS) -o serv $^

client: client.o
	@echo Compiling client
	$(CC) $(CFLAGS) -o cli $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	@echo removing files
	-rm *.o

distclean: clean
	-rm serv cli
