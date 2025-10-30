build:
	clang -o fsc src/*.c --std=c23 -D_XOPEN_SOURCE=800
