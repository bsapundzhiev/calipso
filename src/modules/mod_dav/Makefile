
NAME    =       mod_dav
SRCS    =       mod_dav.c
OBJS    =       $(SRCS:.c=.o)

CFLAGS  +=  -g -O2 -Wall -D_MODULE -DDEBUG -I ../../include -D_FILE_OFFSET_BITS=64
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
