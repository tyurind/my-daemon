/* strncpy example */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
// int main ()
// {
//   char str1[] = "* * * * * cmd";
//   char *str2;//[40];
//   char str3[40];

//   /* copy to sized buffer (overflow safe): */
//   // strncpy ( str2, str1, sizeof(str2) );

//   /* partial copy (only 5 chars): */
//   strncpy ( str3, str1, 5 );
//   str3[2] = '\0';  //  null character manually added 

//   puts (str1);
//   puts (str3);

//   return 0;
// }



void var(char *format, ...)
{
  va_list ap;
  va_start(ap, format);



  if(!strcmp(format, "%d"))
  {
    int x = va_arg (ap, int);
    printf ("You passed decimal object with value %d\n", x);
  }
 
  if(!strcmp(format, "%s"))
  {
    char *p = va_arg (ap, char *);
    printf ("You passed c-string \"%s\"\n", p);
  }
  va_end (ap);
}


// int main(void)
// {
//   var("%d", 255);
//   var("%s", "test string");
//   return 0;
// }


// int main1()
// {
//    const char haystack[20] = "TutorialsPoint";
//    const char needle[10] = "Po";
//    char *ret;

//    ret = strstr("Tutorials %s Point", "%s");

//    printf("The substring is: %s\n", ret);
   
//    return(0);
// }

int main ()
{
   const char dest[] = "123456789";
   const char src[]  = "newstring";

   printf("Before memmove dest = %s, src = %s\n", dest, src);
   memmove(dest, src, 1);
   printf("After memmove dest = %s, src = %s\n", dest, src);

   return(0);
}