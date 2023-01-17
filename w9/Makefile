run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!

app: server client
	@echo Compiling 'server client' complete!
server: server.o
	gcc -Wall -Wextra -g -Iinclude -o src/server src/server.o lib/libj.a -Llib
	@echo Compiling 'server' complete!
server.o:
	gcc -Wall -Wextra -g -Iinclude -c src/server.c  -o src/server.o
client: client.o
	gcc -Wall -Wextra -g -Iinclude -o src/client src/client.o lib/libj.a -Llib
	@echo Compiling 'client' complete!
client.o:
	gcc -Wall -Wextra -g -Iinclude -c src/client.c  -o src/client.o
svtest:
	./src/server 5500
cltest:
	./src/client 127.0.0.1 5500
clean:
	rm -f src/client src/client.o src/server.o src/server