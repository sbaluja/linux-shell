myShell: myShell.c
	gcc -std=gnu99 -Wpedantic myShell.c -o myShell

clean:
	rm -i myShell