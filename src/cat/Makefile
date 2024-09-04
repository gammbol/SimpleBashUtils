COMPILATION = gcc -Wall -Werror -Wextra
NAME = s21_cat
SOURCE = main.c

all: $(NAME)

$(NAME): $(SOURCE) 
	$(COMPILATION) $(SOURCE) -o $(NAME)

clang-check:
	cp ../../materials/linters/.clang-format .
	clang-format -n $(SOURCE)
	rm -rf .clang-format

clang-correct:
	cp ../../materials/linters/.clang-format .
	clang-format -i $(SOURCE)
	rm -rf .clang-format

clean:
	rm $(NAME)

clean_log:
	rm -rf cat.log s21_cat.log

test: $(NAME)
	sh tests.sh

rebuild: clean all
