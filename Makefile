build:
	gcc map_reduce.c utils.c -o map_reduce -lm -lpthread -Wall -Werror
clean:
	rm -rf map_reduce
