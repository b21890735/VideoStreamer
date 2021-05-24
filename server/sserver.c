
#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "framebuffer.h"
#include "util.h"

#define BUFFER_SIZE 50
#define FPS 20

#define CHANNEL_ON 1
#define CHANNEL_OFF 0


typedef struct{
    char *name;
    int status;
    char *file;
    Queue *queue;
    pthread_mutex_t mutex;
    sem_t *sem_full;
    sem_t *sem_half;
    int stream_order;
    Frame *stream_frame;
} Channel;


long int port, streams;

//there are 3 channels
Channel chs[3];

pthread_t workers[10];

//Function signatures
int argument_parser(int argc , char *argv[]);
void print_channel_info();
void *socket_server();
void *producer_func(void *c);
void *channel_func(void *c);
void *consumer_func(void *c);


int main(int argc , char *argv[]){
	// Parse arguments
	int arg_parse_result = argument_parser(argc, argv);

	// If arguments are not valid, print error and exit
	if(!arg_parse_result){
		printf("Insufficient or wrong set of arguments Supplied. Please fix and retry.\n");
		return 1;
	}

	// Print details received from command line
	printf("Port: %ld\n", port);
	printf("# of Streams: %ld\n", streams);
	//print channel info
	print_channel_info();

	//declare threads for socket_listener, channels and producers (in size of number of streams)
	pthread_t socket_listener;
	pthread_t producers[streams];
	pthread_t channels[streams];

    //initialize socket listener thread
	pthread_create(&socket_listener, NULL, (void *)socket_server, NULL);

	// create & join producer threads creation
	for(int i = 0; i < streams; i++){
		pthread_create(&producers[i], NULL, (void *)producer_func, (void *)&chs[i]);
		pthread_create(&channels[i], NULL, (void *)channel_func, (void *)&chs[i]);
	}

	//used pthread_join to allow the application to wait for threads to terminate.
	for(int i = 0; i < streams; i++){
        pthread_join(producers[i], NULL);
        pthread_join(channels[i], NULL);
    }

	pthread_join(socket_listener, NULL);
	
    

	return 0;
}


int argument_parser(int argc , char *argv[]) {
    //initialized channel's status as OFF
	chs[0].status = CHANNEL_OFF;
	chs[1].status = CHANNEL_OFF;
	chs[2].status = CHANNEL_OFF;

	//iterate through args and find related command line arguments
	for(int i = 1; i < argc; i++){
		char *arg = argv[i];

		//get port number
		if(strcmp(arg, "-p") == 0 && i+1 <= argc){
			char *errstr;
			port = strtol(argv[++i], &errstr, 10);
			if(!port){
				printf("Port Error\n");
				return 0;
			}
		}

        //get number of streams
		if(strcmp(arg, "-s") == 0 && i+1 <= argc){
			char *errstr;
			streams = strtol(argv[++i], &errstr, 10);
			if(!streams){
				printf("Streams Error\n");
				return 0;
			}
		}

        //created two semaphores which are half and full for each channel
        //and give initial values

        //first channel
		if(strcmp(arg, "-ch1") == 0 && i+1 <= argc){
		    //set related fields in the Channel struct
			chs[0].name = "channel1";
			chs[0].status = CHANNEL_ON;
			chs[0].file = argv[++i];

			char *sem_full_name = "sem_ch1_full";
			sem_unlink(sem_full_name);
			chs[0].sem_full = sem_open(sem_full_name, O_CREAT|O_EXCL, S_IRWXU, 0);

			char *sem_half_name = "sem_ch1_half";
			sem_unlink(sem_half_name);
			chs[0].sem_half = sem_open(sem_half_name, O_CREAT|O_EXCL, S_IRWXU, 0);
		}

        //second channel
		if(strcmp(arg, "-ch2") == 0 && i+1 <= argc && streams >= 2){
			chs[1].name = "channel2";
			chs[1].status = CHANNEL_ON;
			chs[1].file = argv[++i];

			char *sem_full_name = "sem_ch2_full";
			sem_unlink(sem_full_name);
			chs[1].sem_full = sem_open(sem_full_name, O_CREAT|O_EXCL, S_IRWXU, 0);

			char *sem_half_name = "sem_ch2_half";
			sem_unlink(sem_half_name);
			chs[1].sem_half = sem_open(sem_half_name, O_CREAT|O_EXCL, S_IRWXU, 0);
		}

        //third channel
		if(strcmp(arg, "-ch3") == 0 && i+1 <= argc && streams == 3){
			chs[2].name = "channel3";
			chs[2].status = CHANNEL_ON;
			chs[2].file = argv[++i];

			char *sem_full_name = "sem_ch3_full";
			sem_unlink(sem_full_name);
			chs[2].sem_full = sem_open(sem_full_name, O_CREAT|O_EXCL, S_IRWXU, 0);

			char *sem_half_name = "sem_ch3_half";
			sem_unlink(sem_half_name);
			chs[2].sem_half = sem_open(sem_half_name, O_CREAT|O_EXCL, S_IRWXU, 0);
		}
	}

    //port and number of streams must be given
	if(!port || !streams){
		printf("Port and Streams arguments should be passed.\n");
		return 0;
	}

    //channel1 must be given
    //but channel2 and channel3 are optional
	if(!chs[0].file){
		printf("Channel 1 should be passed.\n");
		return 0;
	}

	//check if number of streams and channels match
	if(chs[0].status + chs[1].status + chs[2].status != streams){
		printf("Number of streams and channels does not match.\n");
		return 0;
	}


	return 1;
}

//this function prints channel info when they created
void print_channel_info() {
	for(int i = 0; i < streams; i++){
		Channel *ch = chs + i;
		if(ch->status){
			printf("----------------------------------------------------------------------\n");
			printf("Channel Name: %s\n", ch->name);
			printf("Channel File: %s\n", ch->file);
		}
	}
	printf("----------------------------------------------------------------------\n");
}

void *socket_server(){
	//declare socket variables
	int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
     
    //create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);

    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    puts("Socket created");
     
    //Prepare the sockaddr_in struct
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( (int)port );
     
    //bind
    if(bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
     
    //listen socket
    listen(socket_desc , 3);

    //wait for incoming connections
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;

	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ){
        //accept connection
        puts("Connection accepted");
         
        if( pthread_create( &thread_id, NULL,  (void *)consumer_func, (void*) &client_sock) < 0){
            //create thread
            perror("could not create thread");
            return 1;
        }
         
        //now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0){
        perror("accept failed");
        return 1;
    }
}

void *producer_func(void *c) {
    //file pointer for video file input
	FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    //frame rate is given as 20 per seconds
    char *framerate;

    //create a channel
    Channel *ch = (Channel *)c;

    //initialize pthread mutex
    pthread_mutex_init(&ch->mutex, NULL);

    //create buffer
    ch->queue = create_buffer(BUFFER_SIZE);

    //open file to read
    fp = fopen(ch->file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);


    while(1){
    	// go to beginning of the file
        // circular read (continuously)
    	fseek(fp, 0, SEEK_SET);
    	//read all lines in a while loop
	    while ((read = getline(&line, &len, fp)) != -1) {
	        //create a frame
	    	Frame frame;
	    	char *errstr;

	    	//copy line content to frame
	    	strcpy(frame.rate_str, line);
	    	frame.rate = strtol(trim(line), &errstr, 10);
	    	frame.frame[0] = '\0';

	    	//since one frame is 13 lines read all of them
	    	//and put them into frame
	        for(int i = 0; i < 13; i++){
	        	getline(&line, &len, fp);
	        	strncat(frame.frame, line, len);
	    	}

	        //mutex lock
	    	pthread_mutex_lock(&ch->mutex);

	        //if buffer is full, wait for half semaphore
	    	if(is_full(ch->queue)){
	    		sem_wait(ch->sem_half);
	    	}

	    	enqueue(ch->queue, frame);
	    	//decrease semaphore
	    	if(is_full(ch->queue)){
	    		sem_post(ch->sem_full);
	    	}

	    	//unlock mutex
	    	pthread_mutex_unlock(&ch->mutex);

	    }
    }

}

void *channel_func(void *c) {
    //create channel object
	Channel *ch = (Channel *)c;

	int order = 0;

	while(1){
        //until it becomes full, it does not work
		sem_wait(ch->sem_full);
		while(!is_empty(ch->queue)){
		    //get frame from queue
			Frame frame = dequeue(ch->queue);

			//set unique order to make synchronization
			order = (order + 1) % 100;

			//set channel fields from frame
			ch->stream_frame = &frame;
			ch->stream_order = order;

			//wait for frame ratw
			nanosleep((const struct timespec[]){{0, (1e9 * frame.rate)/FPS}}, NULL);

            //when it comes to the half of the buffer size, sem_post half semaphore to other produce it
			if(ch->queue->size < BUFFER_SIZE / 2.0){
				sem_post(ch->sem_half);
			}
		}
		
	}

}

//for every client, it runs thread
void *consumer_func(void *socket_desc) {
    //get socket
	int sock = *(int*)socket_desc;
    int read_size, send_size;
    char *message , client_message[10];
     
    //send greeting messages to the client
    message = "Channel : \n";

    //send data to socket
    send(sock , message , strlen(message), 0);

    //receive data from socket
    //waits for client
    //it is used for being sure that client is connected
    read_size = recv(sock , client_message , 10 , 0);
    printf("%s\n", client_message);

	char *errstr;
    //get channel index
	int ch_index = strtol(client_message, &errstr, 10);

    //find related channel
	Channel *ch = &chs[ch_index - 1];

	int order_buffer = -1;

    //send data
	while(1) {
        // if order buffer != stream order, new frame arrived
		if(order_buffer != ch->stream_order){

            //order used for synchronization
			order_buffer = ch->stream_order;

			send_size = send(sock , ch->stream_frame->rate_str , strlen(ch->stream_frame->rate_str), 0);

			send_size = send(sock , ch->stream_frame->frame , strlen(ch->stream_frame->frame), 0);
			read_size = recv(sock , client_message , 10 , 0);

		}
		nanosleep((const struct timespec[]){{0, 1e9/FPS}}, NULL);

		
		if(read_size == 0) {
	        printf("Client disconnected\n");
	        break;
	    }else if(read_size == -1) {
	        printf("send failed\n");
	        break;
	    }
	}    

}
