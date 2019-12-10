CXX = gcc
ARGS = -lm -O3
EXEC_SIGALRM = sender_sigalrm receiver_sigalrm
EXEC_SELECT = sender_select receiver_select
EXEC_SOCKOPT = sender_sockopt receiver_sockopt
TEST_INPUT = test_in
TEST_OUTPUT = test_sigalrm test_select test_sockopt

all: sigalrm select sockopt

sigalrm: sender_sigalrm.c receiver_sigalrm.c
	$(CXX) -o sender_sigalrm sender_sigalrm.c $(ARGS)
	$(CXX) -o receiver_sigalrm receiver_sigalrm.c $(ARGS)

select: sender_select.c receiver_select.c
	$(CXX) -o sender_select sender_select.c $(ARGS)
	$(CXX) -o receiver_select receiver_select.c $(ARGS)

sockopt: sender_sockopt.c receiver_sockopt.c
	$(CXX) -o sender_sockopt sender_sockopt.c $(ARGS)
	$(CXX) -o receiver_sockopt receiver_sockopt.c $(ARGS)

.PHONY: test clean

test: all

clean:
	rm -rf $(EXEC_SIGALRM) $(EXEC_SELECT) $(EXEC_SOCKOPT) $(TEST_INPUT) $(TEST_OUTPUT)
