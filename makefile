open:
	atom prog6.c
comp:
	gcc -Wall prog6.c -o prog6 -g -lm
clean:
	clear
run:
	./prog6 USC.bmp USC.bmp 63 -17 0
