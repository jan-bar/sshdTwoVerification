#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>

#define SHOW 1                  // 显示回显
#define HIDE 2                  // 关闭回显
#define LENGTH 128              // 输入最大长度
#define TIMEOUT 5               // 输入超时秒数
#define CMP_FILE "/etc/janbar"  // 比较文件

/*
* str: 将标准输入写入str字符串
* display: SHOW->回显,HIDE->不回显
* 返回:
*   0:  正常返回
*   -1: select出现错误
*   -2: 超时
*/
int get_line_text(char *str,const int display)
{
  struct termios oldt, newt;
  if (display == HIDE) { // 设置不回显
    tcgetattr(STDIN_FILENO, &oldt);
    tcgetattr(STDIN_FILENO, &newt);
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  }

  fd_set  input_set;
  FD_ZERO(&input_set);
  FD_SET(STDIN_FILENO, &input_set);
  struct timeval  timeout = {TIMEOUT, 0};

  int retVal = 0;
  int reading = select(1, &input_set, NULL, NULL, &timeout);
  if (reading == 0) {
    retVal = -2;
  } else if (reading == -1) {
    retVal = -1;
  } else {
    fgets(str, LENGTH, stdin);
    char *ch = strrchr(str, '\n');
    if (ch != NULL) {
      *ch = '\0'; // 去掉换行
    }
  }

  if (display == HIDE) { // 恢复之前配置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
  }
  return retVal;
}

int main(int argc, char *argv[])
{
  FILE *fr = fopen(CMP_FILE, "r");
  if (fr == NULL) {
    return 0;
  }
  char cmp_string[LENGTH];
  fgets(cmp_string, LENGTH, fr);
  fclose(fr);

  int login = 0;
  char username[LENGTH], password[LENGTH];
  for (int i = 0;i < 3; i++)
  {
    printf("Username: ");fflush(stdout);
    if (get_line_text(username, SHOW) < 0) {
      return 0;
    }

    printf("Password: ");fflush(stdout);
    if (get_line_text(password, HIDE) < 0) {
      return 0;
    }

    strcat(username, ":");
    strcat(username, password);
    strcat(username, "\n");
    if (strcmp(cmp_string, username) == 0) {
      login = 1; break;
    }
    printf("Login incorrect\n\n");
  }

  if (login == 1) {
    system("/bin/bash");
  }
  return 0;
}