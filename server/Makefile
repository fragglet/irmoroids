
CFLAGS=`pkg-config --cflags irmo` -I..
LDFLAGS=`pkg-config --libs irmo` -lm
OBJS=universe.o main.o server.o

astroserv: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

# $Log: not supported by cvs2svn $

