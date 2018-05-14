CC = gcc
WARNINGS = -Wall -Wextra -Wshadow -Wstrict-prototypes -Wdouble-promotion \
		   -Wjump-misses-init -Wnull-dereference -Wrestrict -Wlogical-op \
		   -Wduplicated-branches -Wduplicated-cond
FILES = main.c erw_error.c erw_tokenizer.c erw_ast.c erw_parser.c erw_scope.c \
		erw_type.c erw_semantics.c vec.c str.c file.c log.c ansicode.c        \
		argparser.c

debug:
	$(CC) $(FILES) $(WARNINGS) -Og -g3 -o compiler

release:
	$(CC) $(FILES) $(WARNINGS) -march=native -mtune=native -O2 -DNDEBUG -o \
		compiler
