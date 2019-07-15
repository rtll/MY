SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:%.c=%.o)
ELF:=client
$(ELF):$(OBJS)
	gcc $^ -o $@ -Wall -pthread -lcrypt
clean:
	rm -rf $(ELF) $(OBJS)
