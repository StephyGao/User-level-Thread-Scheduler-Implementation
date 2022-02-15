lib := libuthread.a
Target := uthread.o queue.o context.o sem.o preempt.o 

FLAGS := -Wall -Wextra -Werror -MMD 
CC := gcc
CCA := ar rcs

all: $(lib)
deps := $(patsubst %.o, %.d, $(Target))
-include $(deps)


$(lib): $(Target)
	$(CCA) -o $@ $^

%.o: %.c
	@echo "Start $@"
	@$(CC) $(FLAGS) -c -o $@ $< 

clean:
	@echo "Clean Done"
	rm -f $(lib) $(Target) $(deps)
