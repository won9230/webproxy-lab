/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

#define test11_9

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, char *method); //11.11 HEAD를 위한 메소드 추가
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method); //11.11 HEAD를 위한 메소드 추가
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);


void doit(int fd)
{
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Requset headers:\n");
  printf("%s",buf);
  sscanf(buf, "%s %s %s",method, uri, version);
  //11.11를 위한 HEAD 추가
  if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD"))  //클라이언트가 다른 메소드를 요청하면 에러 main으로 돌아간서 다음 연결 요청을 기다림
  {
    printf("%d",method);
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio); //GET이면 다른 요청 헤더들을 무시한다.

  is_static = parse_uri(uri, filename, cgiargs);  //요청이 정적 또는 동적 컨텐츠를 위한 것인지 나타내는 플래그
  if (stat(filename, &sbuf) < 0)   //디스크에 없으면 에러 메시지를 클라이언트한테 보내고 리턴
  {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if (is_static)  //정적 컨텐츠를 위한 것이면
  {
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))   //이 파일이 보통 파일이라는 것과 읽기 권한을 가지고 있는지를 검증한다.
    {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
      return;
    }
    serve_static(fd,filename, sbuf.st_size, method);  //맞으면 정적 컨텐츠를 클라이언트에게 제공(11.11 method추가)
  }
  else  //동적 컨텐츠를 위한 것이라면
  {
    if(!(S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode))) //이 파일이 실행 가능한지 검증
    {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs, method);  //맞으면 동적 컨텐츠를 클라이언트에게 제공(11.11 method추가)
  }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, shortmsg);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n",body);

  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Conent-tpye: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Conent-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
  }

void read_requesthdrs(rio_t *rp)  //요청 헤더를 읽고 무시한다.
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  while ((strcmp(buf, "\r\n")))
  {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf); 
  }
  return;
}

int parse_uri(char *uri, char *filename, char * cgiargs)
{
  char *ptr;

  if(!strstr(uri, "cgi-bin"))
  {
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/')
    {
      strcat(filename, "adder.html");
    }
    return 1;
  }
  else
  {
    ptr = index(uri, '?');
    if(ptr)
    {
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    }
    else
    {
      strcpy(cgiargs,"");
    }
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}

#ifdef test11_9
void serve_static(int fd, char *filename, int filesize, char *method) //11.11 HEAD를 위한 메소드 추가
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer : Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnention: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  if(strcasecmp(method, "GET") == 0)
  {
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = malloc(filesize);
    Rio_readn(srcfd, srcp, filesize);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    free(srcp);
  }
}
#else
void serve_static(int fd, char *filename, int filesize, char *method) //11.11 HEAD를 위한 메소드 추가
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer : Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnention: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  srcfd = Open(filename, O_RDONLY, 0);
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  Munmap(srcp, filesize);

}
#endif
void get_filetype(char *filename, char *filetype)
{
  if(strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if(strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if(strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if(strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else if(strstr(filename, ".mpg")) //11.7 숙제
    strcpy(filetype, "video/mpg");
  else if(strstr(filename, ".mp4")) //11.7 숙제
    strcpy(filetype, "video/mp4");
  else
    strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs, char *method)
{
  char buf[MAXLINE], *emptylist[] = { NULL };

  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0)
  {
    setenv("QUERY_STRING", cgiargs, 1);
    //method를 cgi-bin/adder.c에 넘겨주기 위해 환경변수 set
    setenv("REQUEST_METHOD", method, 1);
    Dup2(fd, STDOUT_FILENO);
    Execve(filename, emptylist, environ);
  }
  Wait(NULL);
}

int main(int argc, char **argv) { //반복실행 서버로 명령줄에서 넘겨받은 포트로의 연결 요청을 듣는다.
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);  //듣기 소켓을 오픈한 후 반복적으로 접수하고
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept   트랜잭션을 수행
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,   //자신 쪽의 연결 끝을 닫는다.
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}