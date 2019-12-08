CXX = gcc
ARG = -O3 -o
SRC_FOLDER = src
EXEC_SIGALRM = sender_sigalrm receiver_sigalrm
EXEC_SELECT = sender_select receiver_select
EXEC_SOCKOPT = sender_sockopt receiver_sockopt

all: sigalrm select sockopt

sigalrm: $(SRC_FOLDER)/sender_sigalrm.c $(SRC_FOLDER)/receiver_sigalrm.c
	$(CXX) $(ARG) sender_sigalrm $(SRC_FOLDER)/sender_sigalrm.c
	$(CXX) $(ARG) receiver_sigalrm $(SRC_FOLDER)/receiver_sigalrm.c

select: $(SRC_FOLDER)/sender_select.c $(SRC_FOLDER)/receiver_select.c
	$(CXX) $(ARG) sender_select $(SRC_FOLDER)/sender_select.c
	$(CXX) $(ARG) receiver_select $(SRC_FOLDER)/receiver_select.c

sockopt: $(SRC_FOLDER)/sender_sockopt.c $(SRC_FOLDER)/receiver_sockopt.c
	$(CXX) $(ARG) sender_sockopt $(SRC_FOLDER)/sender_sockopt.c
	$(CXX) $(ARG) receiver_sockopt $(SRC_FOLDER)/receiver_sockopt.c

.PHONY: test clean

test: all

clean:
	rm -rf $(EXEC_SIGALRM) $(EXEC_SELECT) $(EXEC_SOCKOPT)
