CXX = gcc
ARGS = -lm -O3
SRC_FOLDER = src
EXEC_SIGALRM = sender_sigalrm receiver_sigalrm
EXEC_SELECT = sender_select receiver_select
EXEC_SOCKOPT = sender_sockopt receiver_sockopt
TEST_INPUT = test_in
TEST_OUTPUT = test_sigalrm test_select test_sockopt

all: sigalrm select sockopt

sigalrm: $(SRC_FOLDER)/sender_sigalrm.c $(SRC_FOLDER)/receiver_sigalrm.c
	$(CXX) -o sender_sigalrm $(SRC_FOLDER)/sender_sigalrm.c $(ARGS)
	$(CXX) -o receiver_sigalrm $(SRC_FOLDER)/receiver_sigalrm.c $(ARGS)

select: $(SRC_FOLDER)/sender_select.c $(SRC_FOLDER)/receiver_select.c
	$(CXX) -o sender_select $(SRC_FOLDER)/sender_select.c $(ARGS)
	$(CXX) -o receiver_select $(SRC_FOLDER)/receiver_select.c $(ARGS)

sockopt: $(SRC_FOLDER)/sender_sockopt.c $(SRC_FOLDER)/receiver_sockopt.c
	$(CXX) -o sender_sockopt $(SRC_FOLDER)/sender_sockopt.c $(ARGS)
	$(CXX) -o receiver_sockopt $(SRC_FOLDER)/receiver_sockopt.c $(ARGS)

.PHONY: test clean

test: all

clean:
	rm -rf $(EXEC_SIGALRM) $(EXEC_SELECT) $(EXEC_SOCKOPT) $(TEST_INPUT) $(TEST_OUTPUT)
