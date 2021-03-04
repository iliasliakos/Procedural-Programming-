# paths
MODULES = ../../modules
INCLUDE = ../../include

# Compile options. Το -I<dir> λέει στον compiler να αναζητήσει εκεί include files
CFLAGS = -Wall  -g -I$(INCLUDE)

# Αρχεία .o
OBJS = mngstd.o $(MODULES)/UsingHashTable/ADTMap.o $(MODULES)/UsingDoublyLL/ADTBList.o 


# Το εκτελέσιμο πρόγραμμα
EXEC = mngstd

# Παράμετροι για δοκιμαστική εκτέλεση
ARGS = -i input300.txt

$(EXEC): $(OBJS)
	gcc $(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC) $(ARGS)