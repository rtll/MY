SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:%.c=%.o)
ELF:=server
CFLAGS:=$(shell mysql_config --cflags)
$(ELF):$(OBJS)
	gcc $^ -o $@ $(CFLAGS) -pthread -lmysqlclient
clean:
	rm -rf $(ELF) $(OBJS)
