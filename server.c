/* CSD 304 Computer Networks, Fall 2016
	 Lab 4, multicast receiver
	 Team: 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
//#include "gbnpacket.c"
#include <arpa/inet.h>		
#include <unistd.h>		
#include <errno.h>
#include <memory.h>
#include <signal.h>


//SERVER_PORT is used for TCP connection to communicate with the client
//MC_PORT is used for sending the data over UDP
#define MC_PORT 5432
#define BUF_SIZE 4096
#define SERVER_PORT 5897


struct gbnpacket
{
  int type;
  int seq_no;
  int length;
  char data[20000];
};


void DieWithError (char *errorMessage);	
void CatchAlarm (int ignored);

int main(int argc, char * argv[]){
	
	
	
	int opt=1;
	int tcp_port;
	int s; //Descriptor to socket
	struct sockaddr_in sin; /* socket struct */
	char *if_name; /* name of interface */
	struct ifreq ifr; /* interface struct */
	char buf[BUF_SIZE];
	int len;
	/* Multicast specific */
	char *mcast_addr; /* multicast address */
	struct ip_mreq mcast_req;  /* multicast join struct */
	struct sockaddr_in mcast_saddr; /* multicast sender*/
	socklen_t mcast_saddr_len;
	int chunkSize;
	int mcast_portno;
	int flag=0;
	if (argc != 5)        /* Test for correct number of arguments */
	{
		fprintf (stderr,
				 "Error In Usage: %s <Server IP> <TCP port number><Interface No> <Chunk size> \n",
				 argv[0]);
		exit (1);
	}
	
	char *host;
	
	//Take command line arguments from the user
	//The First argument is TCP connection IP address for communication
	//The second argument if TCP port
	// Third argument specifies the interface name to be used.
	//The fourth argument is how large each packed size would be sent(max 20000)
	if (argc==5) {
		host = argv[1];
		tcp_port=atoi(argv[2]);
		if_name = argv[3];
		chunkSize = atoi (argv[4]);   /* Third arg: string to echo */
	}
	
	
	
	
	
	
  struct hostent *hp;
  struct sockaddr_in tcp_socket;
  int tcp_sock;
  int len_tcp;
  /* translate host name into peer's IP address */
  hp = gethostbyname(host);
  if (!hp) {
    fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
    exit(1);
  }
  else
    printf("Client's remote host: %s\n", argv[1]);
  /* build address data structure */
  bzero((char *)&tcp_socket, sizeof(tcp_socket));
  tcp_socket.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&tcp_socket.sin_addr, hp->h_length);
  tcp_socket.sin_port = tcp_port;
  /* active open */
  if ((tcp_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }
  else
    printf("Client created socket.\n");

  if (connect(tcp_sock, (struct sockaddr *)&tcp_socket, sizeof(tcp_socket)) < 0)
    {
      perror("simplex-talk: connect");
      close(tcp_sock);
      exit(1);
    }
  else
    printf("Client connected.\n");
  
  //Ask the client which station it wants to play
  int select_channel;

  channel_select: printf("Enter the channel you want to stream\n");
  scanf("%d",&select_channel);
  //Send the channel selected by client to server to start streaming
  send(tcp_sock,&select_channel,sizeof(select_channel),0);
  //File handler for the file to write the data into
  FILE *write_file;
   
	
	//Files to be streamed have hardcoded sizes.
	//Each station has its own multicast address and port.
	//Currently five stations are supported
	long file_size;
	if(select_channel == 1){
		printf("You selected channel %d\n",select_channel);
		file_size = 4689115;
		mcast_addr = "239.192.16.1";
		write_file = fopen("output_file1.mp4","w");
		fclose(write_file);
		write_file = fopen("output_file1.mp4","a+");
		mcast_portno=MC_PORT;
	}
	else if(select_channel == 2){
		printf("You selected channel %d\n",select_channel);
		file_size = 4490845;
		mcast_addr = "239.192.16.2";
		write_file = fopen("output_file2.mp4","w");
		fclose(write_file);
		write_file = fopen("output_file2.mp4","a+");
		mcast_portno=6000;
	}
	else if(select_channel == 3){
		printf("You selected channel %d\n",select_channel);
		file_size = 4870712;
		mcast_addr = "239.192.16.3";
		write_file = fopen("output_file3.mp4","w");
		fclose(write_file);
		write_file = fopen("output_file3.mp4","a+");
		mcast_portno=6001;
	}
	else if(select_channel == 4){
		printf("You selected channel %d\n",select_channel);
		file_size = 4967996;
		mcast_addr = "239.192.16.4";
		write_file = fopen("output_file4.mp4","w");
		fclose(write_file);
		write_file = fopen("output_file4.mp4","a+");
		mcast_portno=6002;
	}
	else if(select_channel == 5){
		printf("You selected channel %d\n",select_channel);
		file_size = 205034688;
		mcast_addr = "239.192.16.5";
		write_file = fopen("output_file5.mp4","w");
		fclose(write_file);
		write_file = fopen("output_file5.mp4","a+");
		mcast_portno=6003;
	}
	else{
		printf("Please enter a valid value for channel.\n");
		return -1;
	}
	
	


	char *buffer;
	buffer = (char *)malloc(sizeof(char) * file_size);
	
	int recvMsgSize;		
	int packet_rcvd = -1;		
	struct sigaction myAction;
	double lossRate;	

	


	/* create udp socket to recieve the file */
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("receiver: socket");
		exit(1);
	}

	/* build address data structure */
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(mcast_portno);
	
	
	/*Use the interface specified */ 
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name , if_name, sizeof(if_name)-1);
	
	if ((setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, 
			sizeof(ifr))) < 0)
		{
			perror("receiver: setsockopt() error");
			close(s);
			exit(1);
		}

	/* bind the socket */
	if(flag!=1){
	if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("mcast receiver: bind()");
		exit(1);
	}
	}
	myAction.sa_handler = CatchAlarm;
	if (sigfillset (&myAction.sa_mask) < 0)/*setup for the timer */
		DieWithError ("sigfillset failed");
	myAction.sa_flags = 0;
	if (sigaction (SIGALRM, &myAction, 0) < 0)
		DieWithError ("sigaction failed");
	
	
	
	lossRate = 0;
	srand48(123456789);
	
	
	/* Multicast specific code follows */
	/* build IGMP join message structure */
	mcast_req.imr_multiaddr.s_addr = inet_addr(mcast_addr);
	mcast_req.imr_interface.s_addr = htonl(INADDR_ANY);

	/* send multicast join message */
	if ((setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
			(void*) &mcast_req, sizeof(mcast_req))) < 0) {
		perror("mcast join receive: setsockopt()");
		exit(1);
	}

	
	
	int tear_count=0;
	/* receive multicast messages */  
	printf("\nReady to listen!\n\n");

do{ 
	while(1) {
		
		/* reset sender struct */
		memset(&mcast_saddr, 0, sizeof(mcast_saddr));
		mcast_saddr_len = sizeof(mcast_saddr);
		
		//Make struction for storing and examining a packet
		struct gbnpacket currPacket; 
		memset(&currPacket, 0, sizeof(currPacket));
		//Recieve the packet
		recvMsgSize = recvfrom (s, &currPacket, sizeof (currPacket), 0, 
					(struct sockaddr *) &mcast_saddr, &mcast_saddr_len);
		printf("Recveived\n");
		//Convert packet related data to host order
		currPacket.type = ntohl (currPacket.type);
		currPacket.length = ntohl (currPacket.length); 
		currPacket.seq_no = ntohl (currPacket.seq_no);
		
		//Packet type 4 signifies connection termination message
		if (currPacket.type == 4){
			FILE * fp;
			fp = fopen("recv_file.mp4","w");
			fwrite(buffer,file_size,1,fp);
			fclose(fp);
			struct gbnpacket ackmsg;
			ackmsg.type = htonl(8);
			ackmsg.seq_no = htonl(0);
			ackmsg.length = htonl(0);
			if (sendto
					(s, &ackmsg, sizeof (ackmsg), 0,
					 (struct sockaddr *) &mcast_saddr,
					 mcast_saddr_len) != sizeof (ackmsg)){
					 	printf("Error sending connection termination message\n");
						DieWithError ("Error sending connection termination information"); 
			}
			alarm (70000000); //Set large timeout if connection cannot be established
			
			//Try to receive more connection termination messages although one would also work.
			while (1){
				while ((recvfrom (s, &currPacket, sizeof (int)*3+chunkSize, 0,
						(struct sockaddr *) &mcast_saddr,
						&mcast_saddr_len))<0){
					if (errno == EINTR)	/* Timed out  */{
						printf("Timeout started again\n");
						
					}
					else
						;
					}
					if (ntohl(currPacket.type) == 4) /* respond to connection termination */
					{
						
						ackmsg.type = htonl(8);
						ackmsg.seq_no = htonl(0);
						ackmsg.length = htonl(0);
						if (sendto
							(s, &ackmsg, sizeof (ackmsg), 0, /* send connection termination ack */
							(struct sockaddr *) &mcast_saddr,
							mcast_saddr_len) != sizeof (ackmsg))
						{
							printf("Error sending connection termination\n");
						}
						tear_count++;
						if(tear_count==2)
							goto cont_label;
					}
				}
				printf("Dying\n");
				
		}
			else{
				if(lossRate > drand48())
					continue; 
				//printf ("RECEIVED PACKET NUMBER %d\n", currPacket.seq_no);
		
				/* Store data in buffer */
				if (currPacket.seq_no == packet_rcvd + 1){
					packet_rcvd++;
					int buff_offset = chunkSize * currPacket.seq_no;
					//Put packed data into buffer using buffer_offset
					memcpy (&buffer[buff_offset], currPacket.data, 
						currPacket.length);
					fwrite(&buffer[buff_offset],currPacket.length,1,write_file);
					//system("firefox localhost/Radio");
				}
				//printf ("SEND ACK for Packet Number %d\n", packet_rcvd);
				//Send ACK for this packet
				struct gbnpacket currAck; 
				currAck.type = htonl (2); 
				currAck.seq_no = htonl (packet_rcvd);
				currAck.length = htonl(0);
				//Send the acknowledgement
				if (sendto (s, &currAck, sizeof (currAck), 0, 
							(struct sockaddr *) &mcast_saddr,
							mcast_saddr_len) != sizeof (currAck))
					DieWithError
						("sendto() error");
						
			}
	
	}
	cont_label:printf("Press (1) to start streaming again, (2) to exit\n");
	fflush(stdin);
	scanf("%d",&opt);
	if(opt==1){
		goto channel_select;
		flag=1;
	}
	else if(opt==2){
		//Client Exited.
		printf("Thank You.. Exiting..\n");
		exit(0);
	}
	else
		printf("Invalid Selection\n");
	}
	while(opt==1);
	fclose(write_file);	
	//Close the udp socket	
	close(s);
	
	return 0;
}



void
DieWithError (char *errorMessage)
{
	perror (errorMessage);
	exit (1);
}

void
CatchAlarm (int ignored)
{
	exit(0);
}
