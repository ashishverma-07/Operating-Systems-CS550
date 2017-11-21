
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

