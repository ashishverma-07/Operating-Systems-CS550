
To get the root access type:
	sudo bash

To Compile use command:
	make

To Insert module:
	insmod linepipe.ko qsize=N

To Run use command:
	./producer /dev/linepipe	
	./consumer /dev/linepipe
	

To Remove module:
	rmmod linepipe.ko

Answer to TASK B:
When we run the script by giving the conditions, we face issues like deadlock, race conditions, synchronization etc. It is read byte by byte so we get error when multiple producers/consumers try to read/write simultaneously leading to incorrect and incomplete output.
