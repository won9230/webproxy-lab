//GET /cgi-bin/adder?15000&213 HTTP/1.0
/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
  char *buf, *p, *arg1_p, *arg2_p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE], val1[MAXLINE], val2[MAXLINE];
  int n1 = 0, n2 = 0;

  if((buf = getenv("QUERY_STRING")) != NULL)
  {
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1, buf);
    strcpy(arg2, p+1);

    arg1_p = strchr(arg1, '=');
    *arg1_p = '\0';
    strcpy(val1, arg1_p+1);

    arg2_p = strchr(arg2, '=');
    *arg2_p = '\0';
    strcpy(val2, arg2_p+1);

    n1 = atoi(val1);
    n2 = atoi(val2);
  }

  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
  sprintf(content, "%sTHE answer is : %d + %d = %d\r\p\n<p>", content, n1, n2, n1 + n2);
  sprintf(content, "%sThanks for visiting!\r\n", content);

  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-typeL text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);

  exit(0);
}
/* $end adder */