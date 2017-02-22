#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/md5.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "support.h"

/*   Task #1 & task #2
 *   ./obj64/server -p 1234
 *
 */

int lru_size = 0;
int n = 0;
int inCache = 0;
void deque_LRU();
/*
 *   LRU cache, each node represent a file
 */
typedef struct lru_cache_node{
  char * filename;
  char * content;
  int length;
  int num;
  struct lru_cache_node *next;
}lru_node;

static lru_node * head;

/* 
 * given file name look in cache, return that node if found in cache 
 */
lru_node * remove_LRU(char *file){
  if(head == NULL) return NULL; // nothing in cache 
  lru_node * current = head; 
  lru_node * prev = NULL;
  while(current){
    if(strcmp(current->filename,file)==0){
      // find in LRU
      inCache--;
      if(prev == NULL) head = current -> next;
      else prev -> next = current -> next;
      return current;
    }
    prev = current;
    current = current -> next;
  }
  return NULL;
}

/* search thtough list */
lru_node * find_LRU(char *file){
  lru_node * current = head;
  while(current){
    if(strcmp(current->filename,file)==0) 
      return current;
    current = current -> next;
  }
  return NULL;
}

/* save to the end of the list */
void save_LRU(lru_node* nodefile){
  // if cache if full
  if(inCache>=lru_size) deque_LRU();
  inCache++;
  //printf("in save _ LRU \n");
  // else we append to the end
  lru_node * current = head; 
  if(head==NULL)   // if empty
    head = nodefile;  
  else{
    while(current->next){  // go to next
      current = current -> next;
    }
    current->next = nodefile;
  }
  //free(current);
}

/* free the node with least num of the list */
void deque_LRU(){
  // printf("call deque\n");
  lru_node * temp = head;
  lru_node * min = temp;
  if(head == NULL) printf("empty LRU\n");
  //if(temp->next==NULL) min = temp;
  while(temp){
    if(temp->num < min->num){ // temp num < next num
      min = temp;
    }
    temp = temp -> next;
  }    // find the min filename which is least used
  printf("removed from cache: %s\n",min->filename);
  remove_LRU(min->filename);  // use find to deque 
}

/* print what is cache */
void print_lru(){
  lru_node * current = head;
  printf("-----   In lRU cache:\n");
  while(current){
    printf("-----   file:%s  lru:%d\n",current->filename,current->num);
    current = current -> next;
  }
  free(current);
}

/* help() - Print a help message */
void help(char *progname) {
    printf("Usage: %s [OPTIONS]\n", progname);
    printf("Initiate a network file server\n");
    printf("  -l    number of entries in cache\n");
    printf("  -p    port on which to listen for connections\n");
}

/* die() - print an error and exit the program */
void die(const char *msg1, char *msg2) {
    fprintf(stderr, "%s, %s\n", msg1, msg2);
    exit(0);
}

/*
 * open_server_socket() - Open a listening socket and return its file
 *                        descriptor, or terminate the program
 */
int open_server_socket(int port) {
    int                listenfd;    /* the server's listening file descriptor */
    struct sockaddr_in addrs;       /* describes which clients we'll accept */
    int                optval = 1;  /* for configuring the socket */

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("Error creating socket: ", strerror(errno));

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0)
        die("Error configuring socket: ", strerror(errno));

    /* Listenfd will be an endpoint for all requests to the port from any IP
       address */
    bzero((char *) &addrs, sizeof(addrs));
    addrs.sin_family = AF_INET;
    addrs.sin_addr.s_addr = htonl(INADDR_ANY);
    addrs.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr *)&addrs, sizeof(addrs)) < 0)
        die("Error in bind(): ", strerror(errno));

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 1024) < 0)  // backlog of 1024
        die("Error in listen(): ", strerror(errno));

    return listenfd;
}

/*
 * handle_requests() - given a listening file descriptor, continually wait
 *                     for a request to come in, and when it arrives, pass it
 *                     to service_function.  Note that this is not a
 *                     multi-threaded server.
 */
void handle_requests(int listenfd, void (*service_function)(int, int), int param) {
    while (1) {
        /* block until we get a connection */
        struct sockaddr_in clientaddr;
        int clientlen = sizeof(clientaddr);
        int connfd;
        if ((connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen)) < 0)
            die("Error in accept(): ", strerror(errno));

        /* print some info about the connection */
        struct hostent *hp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                           sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hp == NULL) {
            fprintf(stderr, "DNS error in gethostbyaddr() %d\n", h_errno);
            exit(0);
        }
        char *haddrp = inet_ntoa(clientaddr.sin_addr);
        printf("server connected to %s (%s)\n", hp->h_name, haddrp);

        /* serve requests */
        service_function(connfd, param);

        /* clean up, await new connection */
        if (close(connfd) < 0)
            die("Error in close(): ", strerror(errno));
    }
}

/** return char * representation of the checksum */
char * create_checksum(char * content){
  //printf("string: %s\n",content);
  char *out = (char*)malloc(33);bzero(out,34);
  unsigned char digest[MD5_DIGEST_LENGTH];
  int n = 0;
  MD5_CTX md5;
  MD5_Init(&md5);
  MD5_Update(&md5, content, strlen(content));
  MD5_Final(digest,&md5);
  for(;n<16;++n){
    sprintf(&out[n*2],"%02x",(unsigned int)digest[n]);
  }
  return out;
}

/* save to cache */
void save_cache(char * name, char * file, int size){
  lru_node * new = malloc(sizeof(lru_node));
  new -> filename = malloc(strlen(name)+1);
  new -> content = malloc(strlen(file)+1);
  new -> length = size;
  n++;
  new -> num = n;
  //printf("in here\n");
  strcpy(new->filename,name);
  strcpy(new->content,file);
  new -> next = NULL;
  save_LRU(new); 
}

/*
 * file_server() - Read a request from a socket, satisfy the request, and
 *                 then close the connection.
 */
void file_server(int connfd, int lru_size) {
  //printf("lru: %d\n",inCache); print_lru();
  char msg[10];bzero(msg,10);
  read(connfd, msg,10); // read PUT or GET
  printf("Client send: %s",msg);
  char filename[256];bzero(filename,256);
  read(connfd,filename,256); // read file name
  printf("File name: %s\n",filename);

  if(strcmp(msg,"PUT\n")==0){    //       PUT 
    /*if(lru_size){
      lru_node * new = find_LRU(filename);
      if(new){
	printf("found in LRU cache");
	return;
      }
      }*/
    char file_size[256];bzero(file_size,256);
    read(connfd,file_size,256); // read fize size
    int size = atoi(file_size);
    printf("file size: %d\n", size);
    char buffer[size];bzero(buffer,size+1);
    read(connfd,buffer,size);   // read file content
    // printf("received buffer: %s\n",buffer);
    FILE *file = fopen(filename,"w+");
    if(!file) die("error open","cannot write");
    fprintf(file,"%s",buffer);  // write to file
    fclose(file);
    write(connfd,"OK",10);
    // save to lru cache
    if(lru_size) {
      //printf("save attempt\n");
      save_cache(filename,buffer,size);
      //printf("save to LRU cache.\n");
      print_lru();
    } else{
      printf("EOF. PUT complete. terminating connection\n"); exit(0);
    }
  } 
  else if (strcmp(msg,"GET\n")==0){    //    GET
    // try find in cache
    if(lru_size){  // save to 
      lru_node * file_node = find_LRU(filename);
      if(file_node){ // found in cache
        n++;file_node->num = n;  // update num
        write(connfd,"OK",10); printf("OK found in cache\n");
        char size[256];
        sprintf(size,"%d",file_node->length);  // cast LENGTH to char
        write(connfd,size,256);    // write SIZE
        write(connfd,file_node->content,file_node->length); // write CONTENT
        printf("sending file from cache complete.");
      } else die(filename,"didn't find file in LRU cache. terminating.");
      print_lru();
    }
    else{   // save to disk
      FILE * readfile = fopen(filename,"r");
      if(!readfile) die(filename,"cannot found file in server");
      //char buffer[1024]; bzero(buffer,1025);
      //if(!fgets(buffer,sizeof(buffer),readfile)) printf("empty file\n");
      write(connfd,"OK",10);           // write OK
      printf("OK\n");
      fseek(readfile,0,SEEK_END);
      int length = ftell(readfile); printf("file size: %d\n",length);
      fseek(readfile,0,SEEK_SET);
      char size[256];
      sprintf(size,"%d",length);
      write(connfd,size,256);  // write file size 
      //printf("length on GET: %s\n",size);
      char content[length];
      char buffer[1024]; bzero(buffer,1025);
      if(!fgets(buffer,sizeof(buffer),readfile)){
	printf("empty file\n");
      }
      //printf("buffer: %s\n",buffer);
      strcpy(content,buffer);
      while(fgets(buffer,sizeof(buffer),readfile)){
        strcat(content,buffer);
      }
      fclose(readfile);
      // printf("sending str: %s\n",content);
      write(connfd,content,length);  // write file content
      printf("EOF. GET file complete. terminating connection\n"); exit(0); 
    } 
  } 
  else if (strcmp(msg,"PUTC\n")==0){
    char file_size[256];bzero(file_size,256);
    read(connfd,file_size,256); // read fize size
    int size = atoi(file_size);
    printf("file size: %d\n",size);
    char buffer[size]; bzero(buffer,size+1); 
    // read check sum
    char read_sum[33]; bzero(read_sum,34);
    read(connfd,read_sum,33); // read CHECKSUM
    //printf("receive checksum: %s\n",read_sum);
    read(connfd,buffer,size);   // read file content
    //printf("receiving content: %s\n",buffer);
    // match checksum
    char match_sum[33]; bzero(match_sum,34);
    strncpy(match_sum,create_checksum(buffer),33);
    //printf("create checksum: %s\n",match_sum);
    if(strcmp(read_sum,match_sum)){
      die("checksum doesn't match.","rejecting file transmit");
    }
    printf("Checksum matches\n");
    FILE *file = fopen(filename,"w+");
    if(!file) die("error open","cannot write");
    fprintf(file,"%s",buffer);  // write to file
    fclose(file);
    write(connfd,"OKC",10);     // write OK
    if(lru_size) {
      save_cache(filename,buffer,size);
      printf("save to LRU cache.\n");
      print_lru();
    } else{
      printf("EOF. PUT complete. terminating connection\n"); exit(0);
    }
  } 
  else if (strcmp(msg,"GETC\n")==0){
    if(lru_size){  // save to cache
      lru_node * file_node = find_LRU(filename);
      if(file_node){ // found in cache
        n++;file_node->num = n;  // update num
        write(connfd,"OKC",10); printf("OKC found in cache\n");
        char size[256]; bzero(size,257);
        sprintf(size,"%d",file_node->length);  // cast LENGTH to char
        write(connfd,size,256);    // write SIZE
        char sum[33]; bzero(sum,34);
	//printf("file content: %s, file size:%d \n",file_node->content,file_node->length);
	strncpy(sum,create_checksum(file_node->content),33);
        //printf("check sum: %s\n",sum);
        write(connfd,sum,33);     // send check sum
        write(connfd,file_node->content,file_node->length); // write CONTENT
	print_lru();
	printf("sending file from cache complete.");
      } else {
        write(connfd,"NOT",10);
        die(filename,"didn't find file in LRU cache. terminating.");
      }
    } else {    
      FILE * readfile = fopen(filename,"r");
      if(!readfile) {
	write(connfd,"NOT",10);
	die(filename,"cannot found file in server");
      }
      write(connfd,"OKC",10);           // write OK
      printf("OKC\n");
      //char buf[1024]; bzero(buf,1024);
      //fgets(buf,sizeof(buf),readfile);
      //printf("buff %s\n",buf);
      fseek(readfile,0,SEEK_END);
      int length = ftell(readfile);
      fseek(readfile,0,SEEK_SET);
      char size[256];
      sprintf(size,"%d",length);
      write(connfd,size,256);  // write file size 
      char content[length]; bzero(content,length+1);
      char buffer[1024]; bzero(buffer,1025);
      if(!fgets(buffer,sizeof(buffer),readfile))
      	printf("empty file\n");
      strcpy(content,buffer);
      while(fgets(buffer,sizeof(buffer),readfile)){
	strcat(content,buffer);
      }
      fclose(readfile);
      //printf("GETC file content: %d\n",content);
      char sum[33]; bzero(sum,34);
      strncpy(sum,create_checksum(content),33);
      //printf("check sum: %s\n",sum);
      write(connfd,sum,33);           // write CHECKSUM
      //printf("~~~~~~content: \n");
      printf("%s\n",content);
      write(connfd,content,length);  // write file content
      printf("GETC complete. terminating connection\n"); exit(0); 
    }
  }
  else die(msg,"command not recognized");
}



/*
 * main() - parse command line, create a socket, handle requests
 */
int main(int argc, char **argv) {
    /* for getopt */
    long opt;
    /* NB: the "part 3" behavior only should happen when lru_size > 0 */
    // int  lru_size = 0;
    int  port  = 9000;

    check_team(argv[0]);

    /* parse the command-line options.  They are 'p' for port number,  */
    /* and 'l' for lru cache size.  'h' is also supported. */
    while ((opt = getopt(argc, argv, "hl:p:")) != -1) {
        switch(opt) {
          case 'h': help(argv[0]); break;
          case 'l': lru_size = atoi(optarg); break;
          case 'p': port = atoi(optarg); break;
        }
    }
    
    //printf("%d\n",lru_size);
    /* open a socket, and start handling requests */
    int fd = open_server_socket(port);
    handle_requests(fd, file_server, lru_size);

    exit(0);
}


/* TODO: set up a few static variables here to manage the LRU cache of
       files */

    /* TODO: replace following sample code with code that satisfies the
       requirements of the assignment */

    /* sample code: continually read lines from the client, and send them
       back to the client immediately 
    while (1) {
        const int MAXLINE = 8192;
        char      buf[MAXLINE];   /* a place to store text from the client 
        bzero(buf, MAXLINE);

        /* read from socket, recognizing that we may get short counts */
  //  char *bufp = buf;              /* current pointer into buffer */
        //ssize_t nremain = MAXLINE;     /* max characters we can still read */
        //size_t nsofar;                 /* characters read so far */
        //while (1) {
            /* read some data; swallow EINTRs 
            if ((nsofar = read(connfd, bufp, nremain)) < 0) {
                if (errno != EINTR)
                    die("read error: ", strerror(errno));
                continue;
            }
            /* end service to this client on EOF/
            if (nsofar == 0) {
                fprintf(stderr, "received EOF\n");
                return;
            }
            /* update pointer for next bit of reading 
            bufp += nsofar;
            nremain -= nsofar;
            if (*(bufp-1) == '\n') {
                *bufp = 0;
                break;
            }
      }*/

        /* dump content back to client (again, must handle short counts) 
        printf("server received %d bytes\n", MAXLINE-nremain);
        nremain = bufp - buf;
        bufp = buf;
        while (nremain > 0) {
            /* write some data; swallow EINTRs 
            if ((nsofar = write(connfd, bufp, nremain)) <= 0) {
                if (errno != EINTR)
                    die("Write error: ", strerror(errno));
                nsofar = 0;
            }
            nremain -= nsofar;
            bufp += nsofar;
        }
  }*/
