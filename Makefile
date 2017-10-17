CC = gcc
WARNINGS = -Wall -Wextra -Wshadow -Wstrict-prototypes -Wdouble-promotion -Wjump-misses-init -Wnull-dereference -Wrestrict -Wlogical-op -Wduplicated-branches -Wduplicated-cond

debug:
	$(CC) *.c $(WARNINGS) -Og -g3 -o compiler
release:
	$(CC) *.c $(WARNINGS) -march=native -mtune=native -O2 -DNDEBUG -o compiler
