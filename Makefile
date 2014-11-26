bench_rfc2544 :
	gcc -Wall -g3 -I include src/rfc2544-receiver.c -o bin/receiver
	gcc -Wall -g3 -I include src/rfc2544-sender.c -o bin/sender

clean : 
	rm bin/*
