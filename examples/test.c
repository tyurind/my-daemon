/* strncpy example */
#include <stdio.h>
#include <string.h>

int main ()
{
  char str1[] = "* * * * * cmd";
  char *str2;//[40];
  char str3[40];

  /* copy to sized buffer (overflow safe): */
  // strncpy ( str2, str1, sizeof(str2) );

  /* partial copy (only 5 chars): */
  strncpy ( str3, str1, 5 );
  str3[2] = '\0';  //  null character manually added 

  puts (str1);
  puts (str3);

  return 0;
}