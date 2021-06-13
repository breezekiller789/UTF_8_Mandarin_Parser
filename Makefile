CC = gcc
CCFLAGS = -O3
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
CURDIR = $(shell pwd)
EXE = UTF8_Mandarin_Parser
RED = \033[1;31m
GREEN = \033[1;32m
YELLOW = \033[1;33m
BLUE = \033[1;34m
NC = \033[1;0m

$(EXE) : $(SRC)
	@echo "$(RED)Compiling$(NC)"
	$(CC) $(SRC) -o $(EXE)
	@echo "$(GREEN)Done$(NC)"

clean :
	@echo "$(BLUE)Cleaning...$(NC)"
	rm $(EXE)
	@echo "$(YELLOW)Done$(NC)"
