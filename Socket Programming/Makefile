all:
	g++ -ggdb -Wall serverA.cpp -o serverA
	g++ -ggdb -Wall serverB.cpp -o serverB
	g++ -ggdb -Wall aws.cpp -o aws
	g++ -ggdb -Wall client.cpp -o client

.PHONY: serverA serverB aws clean

serverA:
	./serverA

serverB:
	./serverB

aws:
	./aws

clean:
	rm serverA serverB aws client



