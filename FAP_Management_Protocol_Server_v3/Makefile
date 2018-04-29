###############################################################################
#                         User-Aware Flying AP Project
#                       FAP Management Protocol (Server)
###############################################################################
#                        Comunicacoes Moveis 2017/2018
#                             FEUP | MIEEC / MIEIC
###############################################################################

CC		= gcc
CFLAGS	= -Wall -pthread -g -D_GNU_SOURCE #-Wextra

BIN		= bin
SRC		= src
LIB		= lib
TEST	= test

MATH_LIBRARY	= m
TEST_EXECUTABLE	= Test_FapManagementProtocol_Server


.PHONY: all
all: $(BIN)/$(TEST_EXECUTABLE)


.PHONY: run_test
run_test: all
	./$(BIN)/$(TEST_EXECUTABLE)


$(BIN)/$(TEST_EXECUTABLE): $(TEST)/* $(SRC)/* $(LIB)/*
	$(CC) $(CFLAGS) -I$(SRC) -I$(LIB) $(TEST)/*.c $(SRC)/*.c $(LIB)/*/*.c -l$(MATH_LIBRARY) -o $@


.PHONY: clean
clean:
	rm -rf $(BIN)/*
