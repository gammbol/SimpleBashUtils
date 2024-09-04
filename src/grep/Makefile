cc=gcc
cflags=-Wall -Werror -Wextra
name = s21_grep
source = main.c

all: $(name)

$(name): $(source)
	$(cc) $(cflags) $< -o $@

clang-check:
	cp ../../materials/linters/.clang-format .
	clang-format -n $(source)
	rm -rf .clang-format

clang-correct:
	cp ../../materials/linters/.clang-format .
	clang-format -i $(source)
	rm -rf .clang-format

clean:
	rm -rf $(name)

test: $(name)
	sh tests_script_grep.sh

rebuild: clean all
