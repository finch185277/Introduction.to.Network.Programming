CXX = gcc
ARG = -O3 -o
SRC_FOLDER = src
EXEC_SIGALRM = sender_sigalrm receiver_sigalrm
EXEC_SELECT = sender_select receiver_select
EXEC_SOCKOPT = sender_sockopt receiver_sockopt

all: sigalrm select sockopt

sigalrm: $(SRC)/sender_sigalrm.c $(SRC)/receiver_sigalrm.c
	$(CXX) $(ARG) sender_sigalrm $(SRC)/sender_sigalrm.c
	$(CXX) $(ARG) receiver_sigalrm $(SRC)/receiver_sigalrm.c

select: $(SRC)/sender_select.c $(SRC)/receiver_select.c
	$(CXX) $(ARG) sender_select $(SRC)/sender_select.c
	$(CXX) $(ARG) receiver_select $(SRC)/receiver_select.c

sockopt: $(SRC)/sender_sockopt.c $(SRC)/receiver_sockopt.c
	$(CXX) $(ARG) sender_sockopt $(SRC)/sender_sockopt.c
	$(CXX) $(ARG) receiver_sockopt $(SRC)/receiver_sockopt.c

.PHONY: test clean

test: all

clean:
	rm -rf $(EXEC_SIGALRM) $(EXEC_SELECT) $(EXEC_SOCKOPT)
