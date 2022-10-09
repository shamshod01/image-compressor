#---------------------------------------------------------------
#
#  4190.308 Computer Architecture (Fall 2022)
#
#  Project #1: 
#
#  September 6, 2022
#
#  Seongyeop Jeong (seongyeop.jeong@snu.ac.kr)
#  Jaehoon Shim (mattjs@snu.ac.kr)
#  IlKueon Kang (kangilkueon@snu.ac.kr)
#  Wookje Han (gksdnrwp@snu.ac.kr)
#  Jinsol Park (jinsolpark@snu.ac.kr)
#  Systems Software & Architecture Laboratory
#  Dept. of Computer Science and Engineering
#  Seoul National University
#
#---------------------------------------------------------------


TARGET = pa1
SRCS = main.c pa1.c
CC = gcc
CFLAGS = -g -O2 -Wall 
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ 

clean:
	$(RM) $(TARGET) $(OBJS)

