CONTAINER_OBJECTS = container.cpp
SOCKETS_OBJECTS = sockets.cpp

all: container sockets

container: $(CONTAINER_OBJECTS)
	g++ $(CONTAINER_OBJECTS) -o container
sockets: $(SOCKETS_OBJECTS)
	g++ $(SOCKETS_OBJECTS) -o sockets
clean: 
	rm -f *.o container sockets