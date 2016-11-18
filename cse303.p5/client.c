#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
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

/*  Task #1
 *  ./obj64/client -P more.txt -s 128.180.120.87 -p 1234
 *  ./obj64/client -G more.txt -s 128.180.120.87 -p 1234 -S GETfile.txt
 *
 *  Task #2
 *  ./obj64/client 
 *  ./obj64/client -G more.txt -C -s 128.180.120.87 -p 1234 -S getc.txt
 *
 *
 */

int checksum = 0;
int encryption = 0;

/*
 * die() - print an error and exit the program
 */
void die(const char *msg1, char *msg2) {
    fprintf(stderr, "%s, %s\n", msg1, msg2);
    exit(0);
}

/* encrypt message */
char * my_encrypt(char * content){ 
  RSA* key;
  int result,key_size;
  FILE * key_file = fopen("public.pem","r");
  if(!key_file) die("public.pem","cannot open key file");
  key = PEM_read_RSA_PUBKEY(key_file,NULL,NULL,NULL);
  if(!key) die("cannot read key","terminating");
  // encrypt data
  key_size = RSA_size(key);
  char * encrypt_msg = malloc(key_size+1);
  bzero(encrypt_msg,key_size+1);
  result = RSA_public_encrypt(key_size,(unsigned char*)content,(unsigned char*)encrypt_msg,key,RSA_NO_PADDING);
  if(result<0) die("enrypt wrong","terminating");
  fclose(key_file);
  RSA_free(p_rsa);
  return encrypt_msg;
}

char * decrypt(char* content){
  RSA* key;
  int result;key_size;
  FILE * key_file = fopen("private.pem","r");
  if(!key_file) die("private.pem","cannot open key file");
  key = PEM_read_RSAPrivateKey(key_file,NULL,NULL,NULL);
  if(!key) die("cannot read key","terminating");
  // decrypt data
  key_size = RSA_size(key);
  char * decrypt_msg = malloc(key_size+1);
  bzero(encrypt_msg,key_size+1);
  result = RSA_private_decrypt(key_size,(unsigned char*)content,(unsigned char*)decrypt_msg,key,RSA_NO_PADDING);
  if(result<0) die("enrypt wrong","terminating");
  fclose(key_file);
  RSA_free(p_rsa);
  return decrypt_msg;
}
/*
 * help() - Print a help message
 */
void help(char *progname) {
    printf("Usage: %s [OPTIONS]\n", progname);
    printf("Perform a PUT or a GET from a network file server\n");
    printf("  -P    PUT file indicated by parameter\n");
    printf("  -G    GET file indicated by parameter\n");
    printf("  -C    use checksums for PUT and GET\n");
    printf("  -e    use encryption, with public.pem and private.pem\n");
    printf("  -s    server info (IP or hostname)\n");
    printf("  -p    port on which to contact server\n");
    printf("  -S    for GETs, name to use when saving file locally\n");
}


/*
 * connect_to_server() - open a connection to the server specified by the
 *                       parameters
 */
int connect_to_server(char *server, int port) {
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;
    char errbuf[256];                                   /* for errors */

    /* create a socket */
    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("Error creating socket: ", strerror(errno));

    /* Fill in the server's IP address and port */
    if ((hp = gethostbyname(server)) == NULL) {
        sprintf(errbuf, "%d", h_errno);
        die("DNS error: DNS error ", errbuf);
    }
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0],
          (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    /* connect */
    if (connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        die("Error connecting: ", strerror(errno));
    return clientfd;
}

/*
 * echo_client() - this is dummy code to show how to read and write on a
 *                 socket when there can be short counts.  The code
 *                 implements an "echo" client.
 */
void echo_client(int fd) {
    // main loop
    while (1) {
        /* set up a buffer, clear it, and read keyboard input */
        const int MAXLINE = 8192;
        char buf[MAXLINE];
        bzero(buf, MAXLINE);
        if (fgets(buf, MAXLINE, stdin) == NULL) {
            if (ferror(stdin))
                die("fgets error", strerror(errno));
            break;
        }

        /* send keystrokes to the server, handling short counts */
        size_t n = strlen(buf);
        size_t nremain = n;
        ssize_t nsofar;
        char *bufp = buf;
        while (nremain > 0) {
            if ((nsofar = write(fd, bufp, nremain)) <= 0) {
                if (errno != EINTR) {
                    fprintf(stderr, "Write error: %s\n", strerror(errno));
                    exit(0);
                }
                nsofar = 0;
            }
            nremain -= nsofar;
            bufp += nsofar;
        }

        /* read input back from socket (again, handle short counts)*/
        bzero(buf, MAXLINE);
        bufp = buf;
        nremain = MAXLINE;
        while (1) {
            if ((nsofar = read(fd, bufp, nremain)) < 0) {
                if (errno != EINTR)
                    die("read error: ", strerror(errno));
                continue;
            }
            /* in echo, server should never EOF */
            if (nsofar == 0)
                die("Server error: ", "received EOF");
            bufp += nsofar;
            nremain -= nsofar;
            if (*(bufp-1) == '\n') {
                *bufp = 0;
                break;
            }
        }

        /* output the result */
        printf("%s", buf);
    }
}

char* create_checksum(char * content){
  //printf("string: %s\n",content);
  char *out = (char*)malloc(33);bzero(out,33);
  int n = 0;
  unsigned char digest[MD5_DIGEST_LENGTH];
  MD5_CTX md5;
  MD5_Init(&md5);
  MD5_Update(&md5,content,strlen(content));
  MD5_Final(digest,&md5);
  for(;n<16;++n){
    sprintf(&out[n*2],"%02x",(unsigned int)digest[n]);
  }
  return out;
  //write(fd,digest,8192);
}

/*
 * put_file() - send a file to the server accessible via the given socket fd
 */
void put_file(int fd, char *put_name) {
    /* TODO: implement a proper solution, instead of calling the echo() client */
  char str[8192]; bzero(str,8192);
  if(checksum){
    write(fd,"PUTC\n",10);
    printf("PUTC\n");
  } else {
    write(fd,"PUT\n",10);
    printf("PUT\n");
  }
  FILE *file = fopen(put_name,"r");
  if(!file) die("open file failed","cannot read file");
  write(fd,put_name,256);
  printf("file name: %s\n",put_name);
  fseek(file,0,SEEK_END);
  int length = ftell(file);  // find length of sending FILE
  fseek(file,0,SEEK_SET);  
  printf("file size: %d\n",length);
  char len[256];bzero(len,256);
  sprintf(len,"%d",length);
  write(fd,len,256);        // write LENGTH
  char buffer[1024];bzero(buffer,1024);
  fgets(buffer,sizeof(buffer),file); // read file
  strcpy(str,buffer); // copy to buffer
  while(fgets(buffer,sizeof(buffer),file)){
    strcat(str,buffer);
  }
  if(encryption){
  // encrypt str
    char encrypt_str[8192]; bzero(encrypt_str);
    strncpy(encrypt_str,my_encrypt(str));
    if(checksum){
      char sum[33];bzero(sum,33);
      printf("encrypting file ...\n");
      strncpy(sum,create_checksum(encrypt_str),33);
      //printf("check sum: %s\n",sum);
      write(fd,sum,33); // write check sum 
    }
    printf("encrypting file ...\n");
    write(fd,encrypt_str,8192);    // write FILE content
    fclose(file);  // close file
    char response[10];
    read(fd,response,10);   // read RESPONSE
    printf("%s\n",response);
  } 
  else {    // normal no encryption 
  if(checksum){
    char sum[33];bzero(sum,33);
    strncpy(sum,create_checksum(str),33);
    //printf("check sum: %s\n",sum);
    write(fd,sum,33); // write check sum 
  }
  write(fd,str,8192);    // write FILE content
  fclose(file);  // close file
  char response[10];
  read(fd,response,10);   // read RESPONSE
  printf("%s\n",response);
 }
}
/*
 * get_file() - get a file from the server accessible via the given socket
 *              fd, and save it according to the save_name
 */
void get_file(int fd, char *get_name, char *save_name) {
    /* TODO: implement a proper solution, instead of calling the echo() client */
    // echo_client(fd);
  if(checksum){
    write(fd,"GETC\n",10); printf("GETC\n");
  } else{
    write(fd,"GET\n",10);      // write GET
    printf("GET\n");    
  }
  write(fd,get_name,256);    // write File Name
  printf("file name: %s\n",get_name);
  char response[10];bzero(response,10);
  read(fd,response,10);    // read OK
  printf("received: %s\n",response);
  if(checksum){
    if(strcmp(response,"OKC"))
      die(get_name,"server failed finding file");
  } else{
    if(strcmp(response,"OK"))   // if not OK die
      die("didn't get file","server failed finding file");
  }
  char length[256];bzero(length,256);
  read(fd,length,256);   // read file size
  printf("file size: %s\n",length);
  int size = atoi(length);
  char buffer[size];bzero(buffer,size+3);
  char decrypt_str[size];bzero(decrypt_str); // for encryption
  if(checksum){
    char read_sum[33]; bzero(read_sum,33);
    read(fd,read_sum,33);  // read CHECKSUM
    //printf("received checksum: %s\n",read_sum);
    read(fd,buffer,size);   // read FILE CONTENT
    //printf("file contents: %s\n",buffer);
    char match_sum[33]; bzero(match_sum,33);

    //------ decrypt received file
    if(encryption){
      printf("decrypting file ...\n");
      strncpy(decrypt_str,my_encrypt(buffer));
      strncpy(match_sum,create_checksum(decrypt_str),33);
    }
    else{
      strncpy(match_sum,create_checksum(buffer),33);
    }
    //printf("create checksum:%s\n",match_sum);
    if(strcmp(read_sum,match_sum)){
      die("checksum doesn't match","rejecting file transmit");
    }
  } else{
    read(fd,buffer,size);  // read content
    //------ decrypt received message
    if(encryption){
      printf("decrypting file ...\n");
      char decrypt_str[size]; bzero(decrypt_str);
      strncpy(decrypt_str,my_encrypt(buffer));
    }
  }
  if(checksum) printf("Checksum match\n");
  FILE *file = fopen(save_name,"w+");
  if(!file) die("failed to open file","cannot write to");
  if(encryption) fprintf(file,"%s",decrypt_str);
  else fprintf(file,"%s",buffer);
  fclose(file);
  printf("saved file: %s\n",save_name);
}

/*
 * main() - parse command line, open a socket, transfer a file
 */
int main(int argc, char **argv) {
    /* for getopt */
    long  opt;
    char *server = NULL;
    char *put_name = NULL;
    char *get_name = NULL;
    int   port;
    char *save_name = NULL;

    check_team(argv[0]);

    /* parse the command-line options. */
    /* TODO: add additional opt flags */
    while ((opt = getopt(argc, argv, "hs:P:G:CeS:p:")) != -1) {
      switch(opt) {
        case 'h': help(argv[0]); break;
        case 's': server = optarg; break;
        case 'P': put_name = optarg; break;
        case 'C': checksum = 1; break;
        case 'e': encryption = 1; break;
        case 'G': get_name = optarg; break;
        case 'S': save_name = optarg; break;
        case 'p': port = atoi(optarg); break;
      }
    }

    /* open a connection to the server */
    int fd = connect_to_server(server, port);
    //printf("check sums status: %d\n",checksum);
    /* put or get, as appropriate */
    if (put_name)
        put_file(fd, put_name);
    else
        get_file(fd, get_name, save_name);

    /* close the socket */
    int rc;
    if ((rc = close(fd)) < 0)
        die("Close error: ", strerror(errno));
    exit(0);
}
