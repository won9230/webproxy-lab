CC = gcc
CFLAGS = -02 -Wall -I ..

csapp.o: csapp.clean
   $(CC) $(CFLAGS) -c csapp.c
   $(CC) $(CFLAGS) -c echo.c
   $(CC) $(CFLAGS) -c echoclient.c
   $(CC) $(CFLAGS) -c echoserveri.c
echoserveri:
   $(CC) $(CFLAGS) echoserveri.o echo.o csapp.o -o echoserveri
echoclient:
   $(CC) $(CFLAGS) echoclient.o echo.o csapp.o -o echoserveri

clean:
   rm -f *.o csapp *~
   rm -f *.o echo *~
   rm -f *.o echoclient *~
   rm -f *.o echoserveri *~