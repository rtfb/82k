
OBJ=82k.o

82k: $(OBJ)
	gcc -o $@ $^

t: 82k
	./82k -t

%.o: %.c
	gcc -c -o $@ -std=c99 $<
