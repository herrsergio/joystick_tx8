CC = g++
OBJECTS = msocket.o

tx8_js3 : tx8_js3.c $(OBJECTS)
	$(CC) -ljsw -g -o $@ tx8_js3.c $(OBJECTS) 


msocket.o : msocket.h msocket.cpp
	$(CC) -g -c msocket.cpp

clean:
	rm -rf $(OBJECTS)
