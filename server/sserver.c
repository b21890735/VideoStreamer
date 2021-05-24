/*
	bserver.c stands for Betül Server

*/
#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr

#include "framebuffer.h"
#include "util.h"


#define BUFFER_SIZE 50
#define FPS 20

#define CHANNEL_ON 1
#define CHANNEL_OFF 0


typedef struct 
{
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

Channel chs[3];

pthread_t workers[10];

// Definitions
int argument_parser(int argc , char *argv[]);
void print_channel_details();

void *socket_server();

void *producer_func(void *c);
void *channel_func(void *c);
void *worker_func(void *c);


int main(int argc , char *argv[])
{
	// Parse arguments
	int arg_parse_result = argument_parser(argc, argv);

	// If arguments are not valid, print error and exit
	if(!arg_parse_result){
		printf("Insufficient or wrong set of arguments Supplied. Please fix and retry.\n");
		return 1;
	}

	// Print details
	printf("Port: %ld\n", port);
	printf("# of Streams: %ld\n", streams);
	print_channel_details();

	pthread_t socket_listener;
	pthread_t producers[streams];
	pthread_t channels[streams];


	pthread_create(&socket_listener, NULL, (void *)socket_server, NULL);

	// create & join producer threads
	for(int i = 0; i < streams; i++){
		pthread_create(&producers[i], NULL, (void *)producer_func, (void *)&chs[i]);
		pthread_create(&channels[i], NULL, (void *)channel_func, (void *)&chs[i]);
	}

	for(int i = 0; i < streams; i++){
        pthread_join(producers[i], NULL);
        pthread_join(channels[i], NULL);
    }

	pthread_join(socket_listener, NULL);
	
    

	return 0;
}


int argument_parser(int argc , char *argv[]) {
	chs[0].status = CHANNEL_OFF;
	chs[1].status = CHANNEL_OFF;
	chs[2].status = CHANNEL_OFF;

	for(int i = 1; i < argc; i++){
		char *arg = argv[i];
		
		if(strcmp(arg, "-p") == 0 && i+1 <= argc){
			char *errstr;
			port = strtol(argv[++i], &errstr, 10);
			if(!port){
				printf("Port Error\n");
				return 0;
			}
		}

		if(strcmp(arg, "-s") == 0 && i+1 <= argc){
			char *errstr;
			streams = strtol(argv[++i], &errstr, 10);
			if(!streams){
				printf("Streams Error\n");
				return 0;
			}
		}

		if(strcmp(arg, "-ch1") == 0 && i+1 <= argc){
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


	if(!port || !streams){
		printf("Port and Streams arguments should be passed.\n");
		return 0;
	}

	if(!chs[0].file){
		printf("Channel 1 should be passed.\n");
		return 0;
	}

	if(chs[0].status + chs[1].status + chs[2].status != streams){
		printf("Number of streams and channels does not match.\n");
		return 0;
	}


	return 1;
}

void print_channel_details() {
	
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
	// socket 

	int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( (int)port );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
  
        
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;

	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        if( pthread_create( &thread_id, NULL,  (void *)worker_func, (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
}

void *producer_func(void *c) {
	FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char *framerate;

    Channel *ch = (Channel *)c;

    pthread_mutex_init(&ch->mutex, NULL);

    ch->queue = createQueue(BUFFER_SIZE);

    fp = fopen(ch->file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);


    while(1){
    	// go to beginning
    	fseek(fp, 0, SEEK_SET);
	    while ((read = getline(&line, &len, fp)) != -1) {
	    	Frame frame;
	    	char *errstr;
	    	strcpy(frame.rate_str, line);
	    	frame.rate = strtol(trim(line), &errstr, 10);
	    	frame.frame[0] = '\0';
	        for(int i = 0; i < 13; i++){
	        	getline(&line, &len, fp);
	        	strncat(frame.frame, line, len);
	    	}
	    	
	    	pthread_mutex_lock(&ch->mutex);

	    	if(isFull(ch->queue)){
	    		sem_wait(ch->sem_half);
	    	}

	    	enqueue(ch->queue, frame);
	    	if(isFull(ch->queue)){
	    		sem_post(ch->sem_full);
	    	}
	    	pthread_mutex_unlock(&ch->mutex);

	    	
	    }
    }

}

void *channel_func(void *c) {
	
	Channel *ch = (Channel *)c;

	int order = 0;

	while(1){
		sem_wait(ch->sem_full);
		while(!isEmpty(ch->queue)){
			Frame frame = dequeue(ch->queue);
			
			//printf("%s\n", frame.frame);
			//printf("%s: buffer size: %d \n", ch->name, ch->queue->size);

			order = (order + 1) % 100;

			ch->stream_frame = &frame;
			ch->stream_order = order;
			//printf("%d\n", ch->stream_order);
			nanosleep((const struct timespec[]){{0, (1e9 * frame.rate)/FPS}}, NULL);

			if(ch->queue->size < BUFFER_SIZE / 2.0){
				sem_post(ch->sem_half);

			}
		}
		
	}

}

void *worker_func(void *socket_desc) {

	int sock = *(int*)socket_desc;
    int read_size, send_size;
    char *message , client_message[10];
     
    //Send greeting messages to the client
    message = "Channel : \n";
    send(sock , message , strlen(message), 0);
    

    read_size = recv(sock , client_message , 10 , 0);
    printf("%s\n", client_message);

	char *errstr;
	int ch_index = strtol(client_message, &errstr, 10);

	Channel *ch = &chs[ch_index - 1];

	int order_buffer = -1;

	while(1) {
		if(order_buffer != ch->stream_order){
			order_buffer = ch->stream_order;

			
			//printf("%d\n", ch->stream_frame->rate);
			//printf("%s", ch->stream_frame->rate_str);
			//printf("%s", ch->stream_frame->frame);
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
