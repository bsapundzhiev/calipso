
NAME    =       mod_rewrite
SRCS    =       mod_rewrite.c
		
OBJS    =       $(SRCS:.c=.o)

CFLAGS  +=  -fPIC -g -O2 -Wall -D_MODULE -DDEBUG -I ../../include -I../../cplib/pcre -D_FILE_OFFSET_BITS=64 -D_REENTRANT
#LDFLAGS +=  -L ../../cplib/pcre -lpcre

ifeq ($(OS), Darwin)
	CFLAGS += -I ../../../os/macosx
endif

all:shared

static  :       $(OBJS)
	${CC} -r lib$(NAME).a $(OBJS)
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
