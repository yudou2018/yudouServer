run: app
requestData.o: requestData.cpp requestData.h
	g++ -c requestData.cpp -o requestData.o
myEpoll.o: myEpoll.cpp myEpoll.h
	g++ -c myEpoll.cpp -o myEpoll.o
main.o: main.cpp
	g++ -c main.cpp -o main.o
app: requestData.o myEpoll.o main.o
	g++ requestData.o myEpoll.o main.o -o app
.PHONY: run clean
clean:
	rm -f *.o
