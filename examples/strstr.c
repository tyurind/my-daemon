/*
Данная программа выводит сообщение

от невпроворот
 */

#include <stdio.h>
#include <string.h>

int main(void)
{
  char *p;

  p = strstr("хлопот невпроворот", "от");
  printf(p);

  return 0;
}