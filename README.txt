The server part of the project can be runned as:

gcc -std=c99 -lpthread sserver.c -o sserver; 
sserver –p port –s streams –ch1 videofile [–ch2 videofile] [–ch3 videofile]


For example,
./sserver -p 2001 -s 2 -ch1 example.txt -ch2 film.txt

The client part of the project can be runned as:
javac *
java sclient  –a address –p port –ch channelID

For example,
java sclient -a 127.0.0.1 -p 2001 -ch 1

***In my application, more than one client can connect to the same channel. So they can watch the same channel at the same time.

**I implemented all functionalities by myself, did not use any undesired data structures or libraries.

**In the Java (client) part, I have used Java Swing, Jframe. Creted to different Thread classes which are Viewer and Feeder inherited from Thread class

**For the server part (C), diffferent semaphores and threads are used. 

**I created a struct namely 'Channel' for channels and put necessary fields such as buffer, name, status, threads and semaphores.

**The program can be closed by clicking close button in the left corner. When user clicks it, client disconnects from the socket, server realizes that and stop sending data to that client. 

**Unless user closes the program, the server sends data continuously starting from the beginning of the file, and client gets data until it becomes disconnected. You can see connection details on the server side.

**Since multiple clients can watch the stream, if one of them disconnects, other can continue to watch without any delay or problem.

**Inside framebuffer.h file, I implemented Frame and Queue structs for buffer, and implemented necessary functions for buffer inside it.

**I have written a lots of comments inside source files, further details can be understood by reading them.


