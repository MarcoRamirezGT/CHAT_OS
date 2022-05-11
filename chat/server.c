#include <stdio.h>            
#include <stdlib.h>            
#include <sys/socket.h>	 
#include <sys/types.h>            
#include <string.h>      
#include <netinet/in.h> 
#include <pthread.h>     
#include <arpa/inet.h>   
#include <unistd.h>      
#include <signal.h>     
#include "chat.pb-c.h"

#define BACKLOG 100      
#define MAXDATALEN 256   // tamano maximo del mensaje
#define PORT 2012        // puerto por default
 
struct client{  
  int port;            
  char username[10];    
  struct client *next;  // puntero para el siguiente cliente
};

typedef struct client *clients;  
typedef clients head;             // puntero para la primera direccion 
typedef clients addr;             // puntero para la direccion del cliente 

//Funciones
void SendToAll(char *, int connfd);                                    
void SendPrivateMessage (char *msg, char *sender, char *receiver);    // Mensaje privado
void NotifyServerShutdown( );                                         // Notifica que el servidor esta cerrado
head MakeEmpty(head headptr);                                         // Limpia la lista de los usuarios
void RemoveClient( int port, head headptr );                          // Remueve los clientes al momento que se salgan
void InsertClient(int port,char*,head headptr, addr addr_ptr);        // Agregua un nuevo cliente
void DeleteList( head headptr);                                       // Limpia la lista de clientes
void DisplayList( const head headptr);                                // Lista de todos los clientes conectados
void *CloseServer( );                                                 // signal handler 
void *connClientToServer(void * arg);                                 // Con Servidor
void sigBlocktoDisplay();  


int sf2;  
head head_ptr;                // Variable tipo de estructura
char      username[10];       // Tamano del nombre de usuario
char     buffer[MAXDATALEN];  


int main(int argc, char *argv[]) {  
      int                  listenfd, connfd;  // Variables de los sockets
      int                  portnum;           // Variable del puerto
      struct sockaddr_in   server_addr;       // EStructura para la direccion del servidor
      struct sockaddr_in   client_addr;       // Estructura para la direccion del cliente
      socklen_t            cli_size;          // TAmano de la direccion
      pthread_t            thr;               // thread ID
      int                  yes = 1;  
      addr addr_ptr;                          // Estructura addr
      
      printf("\n\t********** SERVIDOR ENCENDIDO **********\n");  
      
  // Puerto opcional
  if( argc == 2 )       
      portnum = atoi(argv[1]);  
  
  else   
      portnum = PORT; // Sino se indica un puerto se usara el puerto por default
      printf("PUERTO NO:\t%d\n",portnum);  
      head_ptr = MakeEmpty(NULL);              		  
      // Server info
      server_addr.sin_family=AF_INET;                      
      server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    //IP address 
      server_addr.sin_port=htons(portnum);  
       printf("DIRECION IP:\t%s\n",inet_ntoa(server_addr.sin_addr));  
      
      // Crear socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);  
  if(listenfd == -1){  
      printf("server- socket() error");	 
      exit(1);  
  } else {  
      printf("socket\t\tcreado.\n");
  }

  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {  
      printf("setsockopt error");	 
      exit(1);  
  } else {
      //printf("reusing\t\tPUERTO\n");
  }  
  
  if(bind(listenfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))==-1){  
      printf("binding failed\n");	
      exit(1);  
  } else {   
      //printf("binding\t\tsuccess.\n\n");
  }

  printf("\t\tPRESIONE CTRL+Z PARA VER LOS CLIENTES EN LINEA\n\n");  
  
  listen(listenfd, BACKLOG);  
  printf("Esperando clientes...\n");  
  if (signal(SIGINT,(void *)CloseServer)==0)	
  if(signal(SIGTSTP, sigBlocktoDisplay)==0)	
  
  // loop for multi client connection 
  while(1){  
      cli_size = sizeof(struct sockaddr_in);                                   // cli_size argumento para la creacion de hilos
      connfd = accept(listenfd, (struct sockaddr *)&client_addr, &cli_size);   // Conexion aceptada
      addr_ptr = head_ptr;  
      
      // Usuario
      bzero(username,10);            
    
    if(recv(connfd, username, sizeof(username),0) >0);  
        username[strlen(username) - 1] = ':';

        printf("\t%d->%s se ha UNIDO al grupo!\n", connfd,username);  
        sprintf(buffer,"%s EN LINEA\n",username);  
        InsertClient(connfd, username, head_ptr, addr_ptr );               
        addr_ptr = addr_ptr->next;  
        // Avisa a todos que se ha conectado
        addr_ptr = head_ptr ;  
          
        do {  
            addr_ptr = addr_ptr->next;  
            sf2 = addr_ptr->port;  
            
            if( sf2 != connfd){  
                send(sf2,buffer ,sizeof(buffer),0);
            }

        } while( addr_ptr->next != NULL );

        printf("ALERTA: Se establecio conexion desde %s & %d\n\n",inet_ntoa(client_addr.sin_addr),connfd); 
            
        struct client args;						
        args.port = connfd;  
        strncpy(args.username, username, 10);  
        pthread_create(&thr,NULL,connClientToServer,(void*)&args);	// Creando un hilo por cada cliente conectado
        pthread_detach(thr);  
  
  } 

  DeleteList(head_ptr);       // Borra a todos los clientes al momento de cerrar el server
  close(listenfd);

  return 0;

}



// Funciones del servidor por cada cliente conectado
void *connClientToServer(void *arguments){  
    struct client *args = arguments;  
    char   buffer[MAXDATALEN], ubuf[50], uname[10];   // buffer for string the server sends   
    char   *strp;            
    char   *msg = (char *) malloc(MAXDATALEN);  
    int    ts_fd,x, bufflen;  
    int    sfd,msglen;  
    ts_fd  = args->port;	
    
    char *recvrname;
    char *buff;
    
    strcpy(uname, args->username);   
    addr   addr_ptr;  
    
    // Listado de clientes en linea
    addr_ptr = head_ptr ;  
    
    do{  
        addr_ptr = addr_ptr->next;  
        sprintf( ubuf," %s esta en LINEA\n",addr_ptr->username );  
        send(ts_fd,ubuf,strlen(ubuf),0);  
    } while( addr_ptr->next != NULL );  
    
    // Chat
    while(1){  
        bzero(buffer,256);  
        bufflen = recv(ts_fd,buffer,MAXDATALEN,0);  
       
        if (bufflen == 0)   
           goto jmp;  
        
        // parse client message 
        printf("Mensaje recibido: %s\n", buffer);

        if (buffer[0] == '@'){
            printf("Mensaje enviado por PRIVADO\n");
            char *spacelocation;
            buff = (char*)malloc(bufflen);
            strcpy(buff, buffer+1);

            int namelen, i = 0;
            recvrname = (char*)malloc(12);  
            spacelocation = strchr(buff, ' ');

            while ((buff + i) != spacelocation){
              strncat(recvrname, buff + i, 1);
              i++;
            }

            recvrname[i+1] = '\0';
            
            printf("EL USUARIO %s RECIBE UN MENSAJE DIRECTO DE %s\n", recvrname, uname);
            namelen = strlen(recvrname);
            
            char* temp = (char*)malloc(strlen(buff));  
            strcpy(temp, buff+namelen);
            printf("El mensaje es: %s\n", temp);

            SendPrivateMessage(temp, uname, recvrname);
            free(temp);
            free(recvrname);

        } else { 
            printf("Mensaje enviado a TODOS.\n");
            /*=if a client quits=*/  
            if ( strncmp( buffer, "quit", 4) == 0 ){  
                jmp:    printf("%d ->%s ha salido de la conversacion, registro guardado\n",ts_fd,uname);  
                sprintf(buffer,"%s ha salido de la conversacion.\n",uname);  
                addr addr_ptr = head_ptr ;  
            
                do{  
                    addr_ptr = addr_ptr->next;  
                    sfd = addr_ptr->port;  
                    if(sfd == ts_fd)   
                     RemoveClient( sfd, head_ptr );  
                    if(sfd != ts_fd)   
                    send(sfd,buffer,MAXDATALEN,0);  
                } while ( addr_ptr->next != NULL );  
            
                DisplayList(head_ptr);  
                close(ts_fd);  
                free(msg);  
                break;  
            }  

            printf("%s %s\n",uname,buffer);  
            strcpy(msg,uname);  
            x = strlen(msg);  
            strp = msg;  
            strp += x;  
            strcat(strp,buffer);  
            msglen = strlen(msg);  
            addr addr_ptr = head_ptr;  
      
      	    do{  
            		addr_ptr = addr_ptr->next;  
            		sfd = addr_ptr->port;   
          	   	if(sfd != ts_fd)   
                	 send(sfd,msg,msglen,0);  
            } while( addr_ptr->next != NULL );  
       

            DisplayList( head_ptr );  
            bzero(msg,MAXDATALEN);
        } 
    } 
    return 0;  

} 


//SendPrivateMessage envia un mensaje privado, el cual como atributo tiene el mensaje, la direccion del que lo envio y el que lo recibe

void SendPrivateMessage (char *msg, char *sender, char *receiver){
    addr  addr_ptr = head_ptr;
    char *usercheck = NULL;
    int   recvrport;
    char *buff = (char*)malloc(MAXDATALEN);
    buff[0] = '\0';
    strncat(receiver, ":", 1);
    strncat(buff, "[De ", 6);
    strncat(buff, sender, strlen(sender));
    strncat(buff, "] ", 2);
    strncat(buff, msg, strlen(msg));
    strncat(buff, "\0", 2);
    printf("Mensaje privado de: %s\n", buff);

    // Obteniendo el puerto del remitente
    do {
        usercheck = addr_ptr->username;
        printf("Buscando al cliente: %s\n", usercheck);

        if(strcmp(usercheck, receiver) == 0){
          printf("Cliente encontrado!\n");
          break;
        } else {
          if (addr_ptr->next == NULL){
            break;
          }

          addr_ptr = addr_ptr->next;
        }

    } while(addr_ptr != NULL);

    recvrport = addr_ptr->port;

    send(recvrport, buff, strlen(buff), 0); 

} 


 
head MakeEmpty( head head_ptr ){  
    if( head_ptr != NULL ) 
          DeleteList(head_ptr);  
          head_ptr = malloc(sizeof(struct client));  
    
    if( head_ptr == NULL )
          printf( "Out of memory!" );  
          head_ptr->next = NULL;  

    return head_ptr;  
} 


void DeleteList( head headptr ){  
    addr addr_ptr, Tmp;  
    addr_ptr = head_ptr->next;   
    headptr->next = NULL;  
    
    while( addr_ptr != NULL ){  
        Tmp = addr_ptr->next;  
        free( addr_ptr );  
        addr_ptr = Tmp;  
    }  
}  
void InsertClient( int port,char *username, head headptr, addr addr_ptr ){  
    addr TmpCell;  
    TmpCell = malloc( sizeof(struct client));  
           
    if( TmpCell == NULL )  
        printf( "Out of space!!!" );  
    
    TmpCell->port = port;  
    strcpy(TmpCell->username,username);  
    TmpCell->next = addr_ptr->next;  
    addr_ptr->next = TmpCell;  
} 
void DisplayList( const head headptr ){  
    addr addr_ptr = headptr ;  
    
    if( headptr->next == NULL ) {
        printf( "No hay usuarios en linea\n" );  
    
    } else { 
        do {  
            addr_ptr = addr_ptr->next;  
            printf( "%d->%s \t", addr_ptr->port,addr_ptr->username );  
        } while( addr_ptr->next != NULL );  
        
        printf( "\n" );  
    }  
} 
void RemoveClient( int port, head headptr ){  
    addr addr_ptr, TmpCell;  
    addr_ptr = headptr;  
    
    while( addr_ptr->next != NULL && addr_ptr->next->port != port )  
       addr_ptr = addr_ptr->next;  
    
    if( addr_ptr->next != NULL ){             
        TmpCell = addr_ptr->next;  
        addr_ptr->next = TmpCell->next;   
        free( TmpCell );  
    }  
} 
void *CloseServer(){        
    printf("\n\nSERVER SHUTDOWN\n");  
    NotifyServerShutdown( );  
    exit(0);  
}  

 
void NotifyServerShutdown(){  
    int sfd;  
    addr addr_ptr = head_ptr ;  
    int i = 0;  
    
    if( head_ptr->next == NULL ) {  
        printf( "EL servidor ha sido vaciado \n\n" );  
        exit(0);  
    
    } else {  
        do{  
            i++;  
            addr_ptr = addr_ptr->next;  
            sfd = addr_ptr->port;  
            send(sfd,"Vuelva pronto",13,0);  
        } while( addr_ptr->next != NULL );  
       

        printf("%d ha sido expulsado\n\n",i);         
    }  
} 


void sigBlocktoDisplay(){  
    printf("\rClientes en linea \n\n");  
    DisplayList(head_ptr);  
}  

 
