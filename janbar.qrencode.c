#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include "qrencode/qrencode.h"

#define SHOW 1                  // 显示回显
#define HIDE 2                  // 关闭回显
#define LENGTH 64               // 输入最大长度
#define CODE_LEN 4              // 验证二维码长度
#define TIMEOUT 60              // 输入超时秒数
#define CMP_FILE "/etc/janbar"  // 密钥文件

// 在标准输出打印一个二维码
// gcc -s -o /bin/janbar janbar.qrencode.c ./qrencode/libqrencode.so.4
int writeQRcodeUTF8(const char *intext)
{
  QRcode *qrcode = QRcode_encodeString(intext, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
  if(qrcode == NULL) {
    return -1;
  }

	int x, y, margin    = 2;
	const char *white   = "";
  const char *reset   = "";
	const char *empty   = " ";
	const char *lowhalf = "\342\226\204";
	const char *uphalf  = "\342\226\200";
	const char *full    = "\342\226\210";
	int realwidth       = (qrcode->width + margin * 2);
  FILE *fp = stdout;

	/* top margin */
	for (y = 0; y < margin/2; y++) {
    fputs(white, fp);
    for (x = 0; x < realwidth; x++)
      fputs(full, fp);
    fputs(reset, fp);
    fputc('\n', fp);
  }

	/* data */
	for(y = 0; y < qrcode->width; y += 2) {
		unsigned char *row1 = qrcode->data + y*qrcode->width;
		unsigned char *row2 = row1 + qrcode->width;

		fputs(white, fp);
		for (x = 0; x < margin; x++) {
			fputs(full, fp);
		}

		for (x = 0; x < qrcode->width; x++) {
			if(row1[x] & 1) {
				if(y < qrcode->width - 1 && row2[x] & 1) {
					fputs(empty, fp);
				} else {
					fputs(lowhalf, fp);
				}
			} else if(y < qrcode->width - 1 && row2[x] & 1) {
				fputs(uphalf, fp);
			} else {
				fputs(full, fp);
			}
		}

		for (x = 0; x < margin; x++)
			fputs(full, fp);
		fputs(reset, fp);
		fputc('\n', fp);
	}
  QRcode_free(qrcode);

	/* bottom margin */
	for (y = 0; y < margin/2; y++) {
    fputs(white, fp);
    for (x = 0; x < realwidth; x++)
      fputs(full, fp);
    fputs(reset, fp);
    fputc('\n', fp);
  }
	return 0;
}

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

// 获取随机加或乘的字符串
int get_rand_string(char *code, int n)
{
  int a = rand()%10+1;
  int b = rand()%10+1;
  int c;
  if ((rand()%2) == 0) {
    c = a * b;
    sprintf(code, "%d * %d = ?", a, b);
  } else {
    c = a + b;
    sprintf(code, "%d + %d = ?", a, b);
  }
  return c;
}

// 大写字母转小写
char *lowercase(char *str)
{
  for (char *p = str; *p != '\0'; p++)
    if (*p >= 'A' && *p <= 'Z')
      *p += 32;
  return str;
}

int main(int argc, char *argv[])
{
  FILE *fr = fopen(CMP_FILE, "r");
  if (fr == NULL) {
    return 0;
  }
  char cmp_string[LENGTH], password[LENGTH], code[LENGTH];
  fgets(cmp_string, LENGTH, fr);
  fclose(fr);
  char *cat_cmp_str = strrchr(cmp_string, '\n');

  int login = 0, ret;
  srand((unsigned int)time(NULL));
  for (int i = 0;i < 3; i++)
  {
    ret = get_rand_string(code, CODE_LEN);
    writeQRcodeUTF8(code);
    sprintf(code, "%d", ret);

    printf("Password: ");fflush(stdout);
    if (get_line_text(password, HIDE) < 0) {
      return 0;
    }

    strcpy(cat_cmp_str, code);
    if (strcmp(lowercase(cmp_string), lowercase(password)) == 0) {
      login = 1; break;
    }
    printf("Login incorrect\n\n");
  }

  if (login == 1) {
    system("/bin/bash");
  }
  return 0;
}
