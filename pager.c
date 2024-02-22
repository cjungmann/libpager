#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <curses.h>  // for tigetstr, tiparm
#include <term.h>

#include "export.h"
#include "termstuff.h"
#include "pager.h"

bool pager_init_flag = false;

EXPORT void pager_init(void)
{
   if (!pager_init_flag)
   {
      ti_start_term();
      pager_init_flag = true;
   }
}

EXPORT void pager_cleanup(void)
{
   if (pager_init_flag)
   {
      ti_cleanup_term();
      pager_init_flag = false;
   }
}

EXPORT void pager_plot_row(DPARMS *parms, int row_index)
{
   // Calculate visible limits
   int first_screen_row = parms->index_row_top;
   int last_screen_row = first_screen_row + parms->line_count-1;

   // Reprint requested line if within visible limits
   if (row_index>=first_screen_row && row_index <= last_screen_row)
   {
      int line = parms->line_top + row_index - parms->index_row_top;
      ti_set_cursor_position(line, parms->chars_left);
      (parms->printer)(row_index,
                        row_index == parms->index_row_focus,
                        parms->chars_count,
                        parms->data_source,
                        parms->data_extra);
   }
}


EXPORT void pager_plot(DPARMS *params)
{
   // You gotta have called pager_init() before star
   assert(ti_values_initialized());
   // Critical but forgettable setting:
   assert(params->printer);

   int left = params->chars_left;
   int line = params->line_top;
   int line_limit = line + params->line_count;
   int chars_count = params->chars_count;

   int row = params->index_row_top;
   int end_row = row + params->line_count;
   if (end_row >= params->row_count)
      end_row = params->row_count-1;

   for (; line < line_limit; ++row, ++line)
   {
      ti_set_cursor_position(line, left);

      // Erase the line before requesting a reprint.
      // This is probably not necessary, plan to delete it.
      ti_printf("\x1b[%dX", chars_count);

      if (row <= end_row)
      {
         bool has_focus = row == params->index_row_focus;
         (params->printer)(row,
                           has_focus,
                           chars_count,
                           params->data_source,
                           params->data_extra);
      }
      // allow erased line above to handle output to vacant rows (rows without data)
   }
}


// EXPORT int pager_begin(DPARMS *parms, KEYMAP *keymap, KEYSTROKE_GETTER ksg)
// {
//    pager_plot(parms);

//    char keybuff[24];
//    ARV arv = ARV_CONTINUE;

//    while(arv != ARV_EXIT)
//    {
//       const char *ks = (*ksg)(keybuff, sizeof(keybuff));
//       if (ks)
//       {
//          arv = (*keymap->kc_func)(parms, keymap, ks);
//          if (arv == ARV_REPLOT_DATA)
//             pager_plot(parms);
//       }
//    }

//    return 0;
// }
