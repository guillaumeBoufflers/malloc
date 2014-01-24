NAME=		libmy_malloc.so

RM=		@rm -f

CC=		gcc -lpthread

OBJ=		malloc.o

CFLAGS=		-fPIC -shared -ggdb

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all
