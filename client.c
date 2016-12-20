
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() and alarm() */
#include <errno.h>      /* for errno and EINTR */
#include <signal.h>     /* for sigaction() */
#include "gbnpacket.c"

#define TIMEOUT_SECS    3   /* Seconds between retransmits */
#define MAXTRIES        10  /* Tries before giving up */
#define SENDER_PORT 5897

int tries = 0;          /* Count of times sent - GLOBAL for signal-handler access */
int base = 0;
int windowSize = 0;
int sendflag = 1;
int tcp_sock2;
void DieWithError (char *errorMessage); /* Error handling function */
void CatchAlarm (int ignored);  /* Handler for SIGALRM */
int max (int a, int b);     /* macros that most compilers include - used for calculating a few things */
int min(int a, int b);      /* I think gcc includes them but this is to be safe */
void station(int channel_data,unsigned short gbnServPort,int chunkSize,int windowSize);
int c_port,c_chunk,w_size;
unsigned short c_tcpsock;


int
main (int argc, char *argv[])
{	
	
	
	if (argc != 5)        /* Test for correct number of arguments */
		{
			fprintf (stderr,
					 "Usage: %s <Server IP> <TCP Server Port No> <Chunk size> <Window Size>\n",argv[0]);
			exit (1);
		}
	
	char *tcp_addr;
	tcp_addr = argv[1];
	int tcp_port=atoi(argv[2]);
	//Create a new TCP socket to communicate with the recievers
	//Get TCP addr and sender port from argv
	struct sockaddr_in tcp_socket;
	int tcp_sock1;
	char str[INET_ADDRSTRLEN];
	int len;

	bzero((char *)&tcp_socket,sizeof(tcp_socket));
	tcp_socket.sin_family = AF_INET;
	tcp_socket.sin_addr.s_addr = inet_addr(tcp_addr);
	tcp_socket.sin_port = tcp_port;
	int mcast_portno;
	
	
	if((tcp_sock1 = socket(PF_INET,SOCK_STREAM,0))<0){
		perror("Simple talk socket");
		exit(1);
	}
	
	inet_ntop(AF_INET,&(tcp_socket.sin_addr),str,INET_ADDRSTRLEN);
	printf("Server is using address %s and port %d\n",str,tcp_port);
	//printf("Here\n");
	if((bind(tcp_sock1,(struct sockaddr *)&tcp_socket,sizeof(tcp_socket)))<0){
		perror("Simple talk bind");
		exit(1);
	}
	else
		printf("Server bind done");
	//printf("Now here\n");
	listen(tcp_sock1,5);
	pid_t pid;
	while(1){
		printf("Waiting to Accept\n");
		if((tcp_sock2 = accept(tcp_sock1,(struct sockaddr *)&tcp_socket,&len))<0){
			perror("Simple talk accept");
			exit(1);
		}
		//printf("Now here 2\n");
		printf("Server listening\n");
		printf("CP1\n");
		int channel_data;
		len = recv(tcp_sock2,&channel_data,sizeof(channel_data),0);
		printf("Channel requested:%d\n",channel_data);
		if(channel_data>=1&&channel_data<=5){
			pid = fork();
			if(pid == 0){
				station(channel_data,atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
			}
			else{
				printf("Parent process\n");
				continue;
				
			}
		}
		else
			printf("Bad channel number\n");
			exit(1);
		
	}
	
	//void station(channel_data){
	
	exit (0);
}

void
CatchAlarm (int ignored)    /* Handler for SIGALRM */
{
	tries += 1;
	sendflag = 1;
}

void
DieWithError (char *errorMessage)
{
	perror (errorMessage);
	exit (1);
}

int
max (int a, int b)
{
	if (b > a)
		return b;
	return a;
}

int
min(int a, int b)
{
	if(b>a)
		return a;
	return b;
}



void station(int channel_data,unsigned short gbnServPort,int chunkSize,int windowSize){
	printf("Inside function\n");
	printf("Channel Data=%d",channel_data);
	long file_size;
	FILE *fp;
	int mcast_portno;
	char *mcast_addr;
	if(channel_data==1){
		mcast_addr = "239.192.16.1";
		fp = fopen("file1.mp4","r");
		fseek(fp,0,SEEK_END);
		file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		mcast_portno=5432;
	}
	else if(channel_data == 2){
		mcast_addr = "239.192.16.2";
		fp = fopen("file2.mp4","r");
		fseek(fp,0,SEEK_END);
		file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		mcast_portno=6000;
	}
	else if(channel_data == 3){
		mcast_addr = "239.192.16.3";
		fp = fopen("file3.mp4","r");
		fseek(fp,0,SEEK_END);
		file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		mcast_portno=6001;
	}
	else if(channel_data == 4){
		mcast_addr = "239.192.16.4";
		fp = fopen("file4.mp4","r");
		fseek(fp,0,SEEK_END);
		file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		mcast_portno=6002;
	}
	else if(channel_data == 5){
		mcast_addr = "239.192.16.5";
		fp = fopen("file5.mp4","r");
		fseek(fp,0,SEEK_END);
		file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		mcast_portno=6003;
	}
	else{
		printf("Bad channel data..Terminating child\n");
		return;
	}
	printf("Mcast port=%d\n",mcast_portno);
	printf("Mcast Addr=%s\n",mcast_addr);
	int sock;         /* Socket descriptor */
	struct sockaddr_in gbnServAddr;   /* Echo server address */
	struct sockaddr_in fromAddr;  /* Source address of echo */
	//unsigned short gbnServPort;   /* Echo server port */
	unsigned int fromSize;    /* In-out of address size for recvfrom() */
	struct sigaction myAction;    /* For setting signal handler */
	char *servIP;         /* IP address of server */
	 int respLen;         /* Size of received datagram */
	int packet_received = -1; /* highest ack received */
	int packet_sent = -1;     /* highest packet sent */
	
	
	
	// Assuming file size if given in variable file_size
	//Assuming name of the file to be sent is file.mp3
	
	//fp = fopen("file.mp4","r");
	//fseek(fp,0,SEEK_END);
	//file_size = ftell(fp);
	//fseek(fp,0,SEEK_SET);
	//char buffer[file_size];
	
	char *buffer;
	buffer = (char *)malloc(sizeof(char) * file_size);
	
	int read_from_file;
	read_from_file = fread(buffer,file_size,1,fp);
	/*if read_from_file!=file_size{
		printf("Error in reading from file\n");
		return -1;
	}*/
 
	const int datasize = file_size;   /* data buffer size */
	//int chunkSize;        /* chunk size in bytes */
	int nPackets = 0;     /* number of packets to send */
	
	
	//mcast_addr = argv[1];
	//chunkSize = atoi (argv[3]);   /* Third arg: string to echo */
	//gbnServPort = atoi (argv[2]); /* Use given port */
	//windowSize = atoi (argv[4]);
	if(chunkSize > 20000)
	{
		fprintf(stderr, "chunk size must be less than 20000\n");
		exit(1);
	}

	nPackets = datasize / chunkSize; 
	if (datasize % chunkSize)
		nPackets++;         /* if it doesn't divide cleanly, need one more odd-sized packet */
	// nPackets--;
	/* Create a best-effort datagram socket using UDP */
	if ((sock = socket (PF_INET, SOCK_DGRAM, 0)) < 0)
		perror ("socket() failed");
	printf ("created socket");

	/* Set signal handler for alarm signal */
	myAction.sa_handler = CatchAlarm;
	if (sigfillset (&myAction.sa_mask) < 0)   /* block everything in handler */
		DieWithError ("sigfillset() failed");
	myAction.sa_flags = 0;

	if (sigaction (SIGALRM, &myAction, 0) < 0)
		DieWithError ("sigaction() failed for SIGALRM");

	/* Construct the server address structure */
	memset (&gbnServAddr, 0, sizeof (gbnServAddr));   /* Zero out structure */
	gbnServAddr.sin_family = AF_INET;
	gbnServAddr.sin_addr.s_addr = inet_addr (mcast_addr); /* Server IP address */
	gbnServAddr.sin_port = htons (mcast_portno);   /* Server port */
	int ctr;
	/* Send the string to the server */
	while ((packet_received < nPackets-1) && (tries < MAXTRIES))
		{
		 // printf ("in the send loop base %d packet_sent %d packet_received %d\n",
		//      base, packet_sent, packet_received);
			if (sendflag > 0)
		{
		sendflag = 0;
			 /*window size counter */
			for (ctr = 0; ctr < windowSize; ctr++)
				{
					packet_sent = min(max (base + ctr, packet_sent),nPackets-1); /* calc highest packet sent */
					struct gbnpacket currpacket; /* current packet we're working with */
					if ((base + ctr) < nPackets)
				{
					memset(&currpacket,0,sizeof(currpacket));
					//printf ("sending packet %d packet_sent %d packet_received %d\n",
											//base+ctr, packet_sent, packet_received);

					currpacket.type = htonl (1); /*convert to network endianness */
					currpacket.seq_no = htonl (base + ctr);
					int currlength;
					if ((datasize - ((base + ctr) * chunkSize)) >= chunkSize) /* length chunksize except last packet */
						currlength = chunkSize;
					else
						currlength = datasize % chunkSize;
					currpacket.length = htonl (currlength);
					memcpy (currpacket.data, /*copy buffer data into packet */
							buffer + ((base + ctr) * chunkSize), currlength);
					if (sendto
							(sock, &currpacket, (sizeof (int) * 3) + currlength, 0, /* send packet */
							 (struct sockaddr *) &gbnServAddr,
							 sizeof (gbnServAddr)) !=
							((sizeof (int) * 3) + currlength))
						DieWithError
							("sendto() sent a different number of bytes than expected");
				}
				}
		}
			/* Get a response */

			fromSize = sizeof (fromAddr);
			alarm (TIMEOUT_SECS); /* Set the timeout */
			struct gbnpacket currAck;
			while ((respLen = (recvfrom (sock, &currAck, sizeof (int) * 3, 0,
									 (struct sockaddr *) &fromAddr,
									 &fromSize))) < 0)
		if (errno == EINTR) /* Alarm went off  */
			{
				if (tries < MAXTRIES)   /* incremented by signal handler */
					{
				printf ("trying again, %d more tries...\n", MAXTRIES - tries);
				break;
					}
				else
					DieWithError ("No Response");
			}
		else
			DieWithError ("recvfrom() failed");

			/* recvfrom() got something --  cancel the timeout */
			if (respLen)
		{
			int acktype = ntohl (currAck.type); /* convert to host byte order */
			int ackno = ntohl (currAck.seq_no); 
			if (ackno > packet_received && acktype == 2)
				{
					//printf ("received ack\n"); /* receive/handle ack */
					packet_received++;
					base = packet_received; /* handle new ack */
					if (packet_received == packet_sent) /* all sent packets acked */
				{
					alarm (0); /* clear alarm */
					tries = 0;
					sendflag = 1;
				}
					else /* not all sent packets acked */
				{
					tries = 0; /* reset retry counter */
					sendflag = 0;
					alarm(TIMEOUT_SECS); /* reset alarm */

				}
				}
		}
		}
	
	for (ctr = 0; ctr < 3; ctr++) /* send teardown packet*/
		{
			struct gbnpacket teardown;
			teardown.type = htonl (4);
			teardown.seq_no = htonl (0);
			teardown.length = htonl (0);
			sendto (sock, &teardown, (sizeof (int) * 3), 0,
					(struct sockaddr *) &gbnServAddr, sizeof (gbnServAddr));
		}
		printf("File sent\n");
		int c_new;
		int len2 = recv(tcp_sock2,&c_new,sizeof(c_new),0);
		printf("Data=%d",c_new);
		if(c_new>=1&&c_new<=5){
			packet_received = -1; 
			packet_sent = -1;    
			nPackets=0;
			base=0;
			ctr=0;
			station(c_new,gbnServPort,chunkSize,windowSize);
		}
		else
			printf("Bad channel selection\n");	
	close (sock); /* close socket */
}
