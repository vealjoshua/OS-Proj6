all: user oss
user: user.c share.h user.h
	gcc -g -o user user.c
oss: oss.c share.h oss.h
	gcc -g -o oss oss.c
make clean: 
	rm -rf *.o *.out user oss
