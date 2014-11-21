bench_rfc2544 :
	gcc -Wall -g3 src/udp-server.c -o bin/server
	gcc -Wall -g3 src/udp-client.c -o bin/client

clean : 
	rm bin/*
