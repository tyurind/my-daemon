/*
 Пример
// Данная программа выводит строку
// из примера
*/
#include <stdio.h>
#include <string.h>

int main(void)
{
  char *p;

  // p = strchr("строка   из примера", ' ');
  p = strstr("1   2   3 4 5 cmd1", " ");

  printf(p);

  return 0;
}