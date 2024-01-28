#include <curses.h>
#include <term.h>

#include <stdio.h>    // printf
#include <unistd.h>   // STDOUT_FILENO
#include <string.h>   // memset
#include <termios.h>  // for tcgetattr, etc

#include <pager.h>

#include "hundred.inc"

int terminal_init(void)
{
   int error_return;
   int result = setupterm((char*)NULL, STDIN_FILENO, &error_return);
   if (result)
   {
      switch(error_return)
      {
         case 1: printf("curses unavailable in hardcopy mode.\n"); break;
         case 0: printf("curses unavailable in generic terminal.\n"); break;
         case -1: printf("curses unavailable, terminfo database missing.\n"); break;
      }
   }
   return result;
}

char get_keyboard_char(void)
{
   struct termios tios_old, tios_raw;
   tcgetattr(STDOUT_FILENO, &tios_old);
   tios_raw = tios_old;
   cfmakeraw(&tios_raw);
   tcsetattr(STDOUT_FILENO, TCSANOW, &tios_raw);
   int c = getchar();
   tcsetattr(STDOUT_FILENO, TCSANOW, &tios_old);
   return c;
}

int simple_line_print(int row_index, int indicated, int length, void *data_source, void *data_extra)
{
   if (row_index < 0 || row_index >= 100)
      return -1;

   if (indicated)
      write(STDOUT_FILENO, "\x1b[7m", 4);

   const char *val = ((const char **)data_source)[row_index];
   int len = dprintf(STDOUT_FILENO,"%-*s", length, val);

   if (indicated)
      write(STDOUT_FILENO, "\x1b[27m", 5);

   // int len = printf("%-*s", length, val);
   // fflush(stdout);
   return len;
}

int main(int argc, const char **argv)
{
   if (terminal_init())
      return 1;
   else
   {
      write(STDOUT_FILENO, "\x1b[2J\x1b[1;1m", 10);
      pager_init();

      DPARMS dparms;
      memset(&dparms, 0, sizeof(dparms));
      pager_init_dparms(&dparms, numbers, 100, simple_line_print, NULL);
      pager_set_margins(&dparms, 4,4,4,4);

      ARV arv = ARV_REPLOT_DATA;
      while(arv != ARV_EXIT)
      {
         if (arv == ARV_REPLOT_DATA)
            pager_plot(&dparms);

         char chr = get_keyboard_char();
         switch(chr)
         {
            case 'q': arv = pager_quit(&dparms); break;
            case 'n': arv = pager_focus_down_one(&dparms); break;
            case 'N': arv = pager_focus_down_page(&dparms); break;
            case 'p': arv = pager_focus_up_one(&dparms); break;
            case 'P': arv = pager_focus_up_page(&dparms); break;
            default: arv = ARV_CONTINUE; break;
         }
      }

      pager_cleanup();
   }

   return 0;
}
