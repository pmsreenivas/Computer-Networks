all:
		gcc -o client client.c 
		gcc -o monitork monitor.c 
		gcc -o awsk aws.c 
		gcc -o serverAk serverA.c 
		gcc -o serverBk serverB.c -lm

.PHONY: serverA
serverA:
		./serverAk	

.PHONY: serverB
serverB:
		./serverBk

.PHONY: aws
aws:
	./awsk

.PHONY: monitor
monitor:
		./monitork					