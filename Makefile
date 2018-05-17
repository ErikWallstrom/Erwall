CC = gcc
WARNINGS = -Wall -Wextra -Wshadow -Wstrict-prototypes -Wdouble-promotion \
		   -Wjump-misses-init -Wnull-dereference -Wrestrict -Wlogical-op \
		   -Wduplicated-branches -Wduplicated-cond -Wstrict-aliasing=1
DEBUG_FLAGS = -Og -g3
RELEASE_FLAGS = -O2 -DNDEBUG -march=native -mtune=native -fstrict-aliasing
FILES = main.c erw_error.c erw_tokenizer.c erw_ast.c erw_parser.c erw_scope.c \
		erw_type.c erw_semantics.c vec.c str.c file.c log.c ansicode.c        \
		argparser.c
EXECUTABLE = compiler

debug:
	$(CC) $(FILES) $(WARNINGS) $(DEBUG_FLAGS) -o $(EXECUTABLE)

release:
	$(CC) $(FILES) $(WARNINGS) $(RELEASE_FLAGS) -o $(EXECUTABLE)
