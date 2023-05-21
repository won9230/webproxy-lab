# echoclient만 파일명에 맞게 바꿔주면 됨.


echoserveri: echoserveri.o echoserveri.c echoclient.o
	gcc -g -o echoserveri echoserveri.o

echoclient : echoclient.o
	gcc -o echoclient echoclient.o


clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f host_info
	rm -f echoserveri