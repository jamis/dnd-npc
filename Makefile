FLAGS=$(CFLAGS) -Iinclude -Ihistory

all: npc.cgi

console: npc

clean:
	rm -f npc.cgi npc src/*.o history/*.o

npc.cgi: src/npccgi.o src/npcEngine.o history/npcHistory.o src/pcgen_interface.o
	gcc -o npc.cgi $(LDFLAGS) -DUSE_COUNTER -DCTRLOCATION="\"/tmp/npc.cnt\"" -lqDecoder -ltemplates -ldndutil src/npccgi.o src/npcEngine.o src/pcgen_interface.o history/npcHistory.o

npc: src/npc_console.o src/pcgen_interface.o src/npcEngine.o history/npcHistory.o
	gcc -o npc $(LDFLAGS) -lwritetem -ldndutil src/npc_console.o src/pcgen_interface.o src/npcEngine.o history/npcHistory.o

src/npccgi.o: src/npccgi.c
	gcc $(FLAGS) -c -o src/npccgi.o src/npccgi.c

src/npcEngine.o: src/npcEngine.c
	gcc $(FLAGS) -c -o src/npcEngine.o src/npcEngine.c

src/pcgen_interface.o: src/pcgen_interface.c
	gcc $(FLAGS) -c -o src/pcgen_interface.o src/pcgen_interface.c

src/npc_console.o: src/npc_console.c
	gcc $(FLAGS) -c -o src/npc_console.o src/npc_console.c

history/npcHistory.o: history/npcHistory.c
	gcc $(FLAGS) -c -o history/npcHistory.o history/npcHistory.c
