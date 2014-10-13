
NAME    =       dummy
SRCS    =       dummy.c
OBJS    =       $(SRCS:.c=.o)

CFLAGS  +=      -D_MODULE -DDEBUG -I ../../include -D_REENTRANT
LDFLAGS +=

ifeq ($(OS), Darwin)
	CFLAGS += -I ../../../os/macosx
endif

all:shared

static  :       $(OBJS)
	${AR} -r lib$(NAME).a $(OBJS)
	ranlib lib$(NAME).a

shared  :       $(OBJS)
ifeq ($(OS), Darwin)
	${CC} -dynamiclib -undefined dynamic_lookup -o $(NAME).bundle $(OBJS)
else
	${CC} -shared -o $(NAME).so $(OBJS)
endif

clean   :
	rm -f $(OBJS)

distclean  :       clean
	rm -f lib$(NAME).a $(NAME).so *~
