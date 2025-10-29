build:
	clang -o fsc src/main.c src/utils.c --std=c23 -D_XOPEN_SOURCE=800
