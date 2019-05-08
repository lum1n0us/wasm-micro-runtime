CC = gcc
#CFLAGS := -Wall -g -Dattr_container_malloc=malloc -Dattr_container_free=free
CFLAGS := -Wall -g -DMALLOC_MEMORY_FROM_SYSTEM

# core
INCLUDE_PATH = -I. -I./coap/er-coap -I./coap/extension -I../../../../../core/iwasm/lib/native-interface/ -I../../../../shared-lib/platform/include -I../../../../shared-lib/platform/linux \
-I../../../../../core/shared-lib/coap/er-coap -I../../../../../core/shared-lib/coap/extension -I../../../../../core/shared-lib/include
App_C_Files := ../../../../../core/iwasm/lib/native-interface/attr-container.c \
               bh_definition.c \
               host_api.c \
               imrt_link_message.c \
               transaction_manager.c \
               ../../../../../core/shared-lib/mem-alloc/bh_memory.c \
               ../../../../../core/shared-lib/coap/er-coap/er-coap.c \
               ../../../../../core/shared-lib/platform/linux/bh_definition.c \
               ../../../../../core/shared-lib/coap/extension/coap_over_tcp.c

OBJS := $(App_C_Files:.c=.o)
LIB = libhostapi.a

all: $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE_PATH) -lpthread -lrt

$(LIB): $(OBJS)
	rm -f $@
	ar cr $@ $(OBJS)
	rm -f $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(LIB)
