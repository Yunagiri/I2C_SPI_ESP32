#$@  	Le nom de la cible
#$< 	Le nom de la première dépendance
#$^ 	La liste des dépendances
#$? 	La liste des dépendances plus récentes que la cible
#$* 	Le nom du fichier sans suffixe
#.PHONY: clean mrproper
CC=g++
CFLAGS= -Wall -I../../include
LDFLAGS= -L../../lib -larduino -lcurl
EXEC=wifi_ESP32
SRC= $(wildcard *.c)
OBJS= $(SRC:.c=.o)

all: $(OBJS)
	$(CC) -o $(EXEC) $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
run: reset
	./$(EXEC)

reset:
	../../sbin/resetgpios

kill:
	../../sbin/killapp $(EXEC)

clean: reset
	rm *.o $(EXEC)
