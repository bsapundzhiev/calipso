
NAME    =       mod_autoind
SRCS    =       mod_autoind.c
OBJS    =       $(SRCS:.c=.o)

CFLAGS  +=      -Wall -g -D_MODULE -DDEBUG -I ../../include -D_FILE_OFFSET_BITS=64 -D_REENTRANT
LDFLAGS +=

all:shared

static  :       $(OBJS)
	${AR} -r lib$(NAME).a $(OBJS)
	ranlib lib$(NAME).a

shared  :       $(OBJS)
	${CC} -shared -o $(NAME).so $(OBJS)

clean   :
	rm -f $(OBJS)

distclean  :       clean
	rm -f lib$(NAME).a $(NAME).so *~
