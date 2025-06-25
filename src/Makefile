CC=g++
CFLAGS=-Wall -Wextra -g --pedantic -I./SFML/include

SFML_LIB_PATH=./SFML/lib
SFML_MODULES=libsfml-graphics.so.3.0.0 libsfml-window.so.3.0.0 libsfml-system.so.3.0.0
LDFLAGS = $(foreach lib,$(SFML_MODULES),$(SFML_LIB_PATH)/$(lib))
LDFLAGS += -Wl,-rpath=./SFML/lib

EXECS=cliente servidor
CLIENT=cliente.cpp
SERVER=servidor.cpp
OBJECTS=Position.o Window.o GameClient.o Message.o NetworkManager.o Socket.o Treasure.o GameServer.o 

all: $(OBJETCS) $(EXECS)

debug: CFLAGS += -DDEBUG
debug: all

servidor: $(SERVER) $(OBJECTS)
	@echo "👷⚒️  Construindo Servidor:" $@
	@$(CC) $(CFLAGS) -o $@ $< $(OBJECTS) $(LDFLAGS)

cliente: $(CLIENT) $(OBJECTS)
	@echo "👷⚒️  Construindo Cliente:" $@
	@$(CC) $(CFLAGS) -o $@ $< $(OBJECTS) $(LDFLAGS)

%.o: %.cpp %.hpp
	@echo "👷⚒️  Construindo Objeto:" $*
	@$(CC) $(CFLAGS) -c $<

veth:
	sudo ip link add veth0 type veth peer name veth1
	sudo ip link set veth0 up
	sudo ip link set veth1 up

clean:
	@echo "Limpando... 🧹🗑️ ✨"
	@rm -f $(OBJECTS)

purge: clean
	@rm -fr $(EXECS) received
