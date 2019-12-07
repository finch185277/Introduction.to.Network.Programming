CXX = gcc
ARG = -O3 -o
EXEC_FILES = sender_sigalrm sender_select sender_sockopt
             receiver_sigalrm receiver_select receiver_sockopt

all: sigalrm select sockopt

sigalrm: sender_sigalrm.c receiver_sigalrm.c
	$(CXX) $(ARG) sender_sigalrm sender_sigalrm.c
	$(CXX) $(ARG) receiver_sigalrm receiver_sigalrm.c

select: sender_select.c receiver_select.c
	$(CXX) $(ARG) sender_select sender_select.c
	$(CXX) $(ARG) receiver_select receiver_select.c

sockopt: sender_sockopt.c receiver_sockopt.c
	$(CXX) $(ARG) sender_sockopt sender_sockopt.c
	$(CXX) $(ARG) receiver_sockopt receiver_sockopt.c

.PHONY: test clean

test: all

clean:
	rm -rf $(EXEC_FILE)
