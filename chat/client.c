#include <stdio.h>  
#include <stdlib.h>        
#include <string.h>     
#include <unistd.h>      
#include <sys/types.h>            
#include <sys/socket.h>          
#include <netinet/in.h>          
#include <pthread.h>          
#include <signal.h>          
#include "chat.pb-c.h"

#define PORTNUM 2012    /* Puerto por default */  
#define MAXDATALEN 256

int	sockfd; 		// socket file descriptor 
int 	n; 			// number of bytes received/sent
struct 	sockaddr_in serv_addr;	// server address  structure 
char   	buffer[MAXDATALEN];  
char   	username[10];       


/*====== Functions ======*/
void *quitproc();  
void *chatwrite(int);  
void *chatread(int);  
void *ctrlzhandler();  
void writelogfile(char str[256]);


FILE *file; // archivo de registro

int main(int argc, char *argv[]){  

	char fname[100];
	int rc;
	time_t temp;
	struct tm *timeptr;
	
	pthread_t thr1,thr2;          //Se crea dos pthreads, uno para leer socket, y otro para escribir
	if( argc != 2 ){       
		printf("Indique la direccion IP\n");  
		exit(0);  
	}  
	// Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);  
	if (sockfd == -1)  
	printf ("error en el socket del cliente\n");  

	
	// Informacion del servidor
	bzero((char *) &serv_addr, sizeof(serv_addr));  
	serv_addr.sin_family = AF_INET;  
	serv_addr.sin_port = htons(PORTNUM);  
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);  
	
	// Nombre del cliente
	bzero(username,10);  
	printf("\nUsuario: ");  
	fgets(username,10,stdin);  
	__fpurge(stdin);       
	username[strlen(username)-1]=':';  
	
	// Configuracion del historial de conversacion, con fecha y usuario
	temp = time(NULL);
	timeptr = localtime(&temp);
	rc = strftime(fname,sizeof(fname),"%b_%d_%Y_Log_File", timeptr);
	fname[rc]='-';
	int i = 0;
	while(i < strlen(username)){
		fname[rc+i+1]=username[i];
		i++;
	}
	fname[rc+i+1]='\0';
	
	file = fopen(fname,"a+"); // Agrega al archivo y si no existe lo crea
	
	//Connect
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))==-1)  
	{  
		printf("Error de conexion\n");  
		exit(0);  
	}  
	else { 
		printf("\n Para enviar un mensaje privado use @nombre_usuario mensaje\n"); 
		printf("%s Conexion exitosa!\n",username);  
		printf("\r Bienvenido al chat grupal : %s",buffer-1);
		 
		send(sockfd,username,strlen(username),0);  


		pthread_create(&thr2,NULL,(void *)chatwrite,(void *)sockfd);	//Hilo para escribir
		pthread_create(&thr1,NULL,(void *)chatread,(void *)sockfd);     //Hilo para leer
		pthread_join(thr2,NULL);  
		pthread_join(thr1,NULL);  
		fclose(file);
	}
	
	return 0;  
} 


void* chatwrite(int sockfd)  
{  
	char fname[100];
	int rc;
	time_t temp;
	struct tm *timeptr;
	while(1)  
	{  
		printf("%s",username);  

		fflush(file);
		fgets(buffer,MAXDATALEN-1,stdin);  
		if(strlen(buffer)-1>sizeof(buffer)){  
			
			bzero(buffer,MAXDATALEN);  
			__fpurge(stdin);  
		}  
		fprintf(file,"%s ",username); 	//Escribe en el registro

		writelogfile(buffer); 		// Escribe el registro de estado de los usuarios
		
		n = send(sockfd,buffer,strlen(buffer),0);  
		if(strncmp(buffer,"quit",4)==0)  
			exit(0);

		bzero(buffer,MAXDATALEN);  
	} 
	return NULL;
}  


void* chatread(int sockfd)  
{  
	// Lee desde el socket

	if(signal(SIGTSTP, (void *)ctrlzhandler)==0)  
	
	while(1)  
	{  
		n=recv(sockfd,buffer,MAXDATALEN-1,0);  
		if(n==0){   
			printf("\nServidor cerrado\n\n");  
			exit(0);  
		}  
		if(n>0){  
			printf("\n%s ",buffer);  
			writelogfile(buffer); 
			fflush(file);
			bzero(buffer,MAXDATALEN);  
		}  
	}

	return NULL;
}  

void *ctrlzhandler(){             
	// Ctrl+z
	printf("\rEscribe 'quit' para salir.\n");

	return NULL;
}

void writelogfile(char str[256]){
	char fname[100];
	int rc;
	time_t temp;
	struct tm *timeptr;
	
	fprintf(file,"%s",str); // Los mensajes recibidos
	
	// Agrega la hora
	temp = time(NULL);
	timeptr = localtime(&temp);
	rc = strftime(fname,sizeof(fname),"%a,%b %d %r", timeptr);
	fprintf(file,"%s\n",fname);
}  
