#include <stdio.h>
#include <string.h>       // strlen()

#include <stdlib.h>       // malloc/free
#include <unistd.h>       // write()

#include <stdarg.h>       // for va_list, etc
#include <alloca.h>

#include <assert.h>
#include <sys/ioctl.h>    // for ioctl() in get_screen_size()

#include <linelist.h>
#include <contools.h>

#include "pager.h"
#include "termstuff.h"

// #include "../lib_cons/cons.h"

typedef struct {
   int   count;
   LLIST *contents;
   const char **lines;
} LLDATA;

const char *get_line_LLDATA(LLDATA *lldata, int index)
{
   assert(index>=0 && index < lldata->count);
   return lldata->lines[index];
}

bool init_LLDATA(LLDATA *lls, const char *filename)
{
   memset(lls, 0, sizeof(LLDATA));
   if (linelist_from_filename(&lls->contents, &lls->count, filename))
   {
      char **temp = (char**)malloc(sizeof(char*) * lls->count);
      if (temp)
      {
         char **sptr = temp;
         char **send = sptr + lls->count;
         LLIST *lptr = lls->contents;

         while (lptr && sptr < send)
         {
            *sptr = lptr->line;

            lptr = lptr->next;
            ++sptr;
         }

         lls->lines = (const char **)temp;

         return true;
      }
      else
      {
         linelist_destroy(&lls->contents);
         lls->count = 0;
      }
   }

   return false;
}

void destroy_LLDATA(LLDATA *lls)
{
   if (lls->lines)
   {
      free(lls->lines);
      lls->lines = NULL;
   }

   if (lls->contents)
      linelist_destroy(&lls->contents);

   lls->count = 0;
}

typedef struct key_map {
   const char *stroke;
   const char *name;
   PACTION action;
} KMAP;

KMAP keys[] = {
   { "q",  NULL,    pager_quit },
   { NULL, "kcud1", pager_focus_down_one },
   { NULL, "kcuu1", pager_focus_up_one },
   { NULL, "knp",   pager_focus_down_page },
   { NULL, "kpp",   pager_focus_up_page },
   { NULL, NULL, NULL}
};

bool keymap_end(KMAP *key) { return key && key->action == NULL; }

void init_keys(KMAP *keys)
{
   KMAP *ptr = keys;
   while (ptr->action)
   {
      if (ptr->stroke==NULL && ptr->name)
      {
         if (!get_termcap_by_name(&ptr->stroke, ptr->name))
         {
            printf("Unable to find cap value from capname '%s'\n", ptr->name);
            exit(1);
         }
      }

      ++ptr;
   }
}

PACTION get_key_action(KMAP *keys, const char *kstr)
{
   KMAP *ptr = keys;
   while ( !keymap_end(ptr))
   {
      if (strcmp(kstr, ptr->stroke)==0)
         return ptr->action;

      ++ptr;
   }

   return NULL;
}

ARV process_keystroke(DPARMS *parms)
{
   char buff[24];
   char *kstroke = get_keystroke(buff, sizeof(buff));
   PACTION action = get_key_action(keys, kstroke);
   if (action)
      return (*action)(parms);
   else
      return ARV_CONTINUE;
}


/**
 * @brief Implementation of line printer function to be used in a DPARMs struct
 *
 * @param "row_index"   index of row in source data of the indicated line
 * @param "indicated"   flag, highlight if *true*, normal if *false*
 * @param "length"      number of characters to print (include spaces to fill line)
 * @param "data_source" to be recase as appropriate data source from which to get data
 * @return Number of characters printed, kind of meaningless.
 */
int lldata_print(int row_index, int indicated, int length, void *data_source)
{
   LLDATA *lldata = (LLDATA*)data_source;
   const char *str = get_line_LLDATA(lldata, row_index);
   if (indicated)
      ti_write_str("\x1b[7m");

   char *buff = (char*)alloca(length+1);
   snprintf(buff, length+1, "%-*s", length, str);
   ti_write_str(buff);
   // ti_printf("%-*s", length, str);

   if (indicated)
      ti_write_str("\x1b[27m");

   return length;
}

void prepare_DPARMS(DPARMS *parms, LLDATA *index)
{
   memset(parms, 0, sizeof(DPARMS));
   pager_init_dparms(parms, index, index->count, lldata_print);
   pager_set_margins(parms, 4, 4, 4, 4);
}

int run_with_keymap(const char *filename)
{
   LLDATA lldata;
   if (init_LLDATA(&lldata, filename))
   {
      DPARMS parms;
      prepare_DPARMS(&parms, &lldata);

      ARV arv = ARV_REPLOT_DATA;
      while (arv != ARV_EXIT)
      {
         if (arv == ARV_REPLOT_DATA)
            pager_plot(&parms);

         arv = process_keystroke(&parms);
      }

      // // Fill screen with 'E's to debug pager coverate
      // ti_write_str("\x1b#8");

      // pager_begin(&parms, (KEYMAP*)&km_test, get_keystroke);

      destroy_LLDATA(&lldata);

      return 0;
   }

   return 1;
}



int main(int argc, const char **argv)
{
   int rval = 0;
   const char *filename = (argc>1 ? argv[1] : "-");

   get_keystroke(NULL, 0);

   pager_init();

   // printf("Pausing to permit deubgger attachment.");
   // get_keystroke(NULL,0);

   // ti_start_term();
   init_keys(keys);


   // ti_hide_cursor();

   rval = run_with_keymap(filename);
   // ti_show_cursor();
   // ti_cleanup_term();

   pager_cleanup();

   return rval;
}
