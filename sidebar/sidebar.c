/**
 * @file
 * GUI display the mailboxes in a side panel
 *
 * @authors
 * Copyright (C) 2004 Justin Hibbits <jrh29@po.cwru.edu>
 * Copyright (C) 2004 Thomer M. Gil <mutt@thomer.com>
 * Copyright (C) 2015-2020 Richard Russon <rich@flatcap.org>
 * Copyright (C) 2016-2017 Kevin J. McCarthy <kevin@8t8.us>
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @page sidebar GUI display the mailboxes in a side panel
 *
 * GUI display the mailboxes in a side panel
 */

#include "config.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mutt/lib.h"
#include "config/lib.h"
#include "email/lib.h"
#include "core/lib.h"
#include "gui/lib.h"
#include "lib.h"
#include "context.h"
#include "format_flags.h"
#include "globals.h"
#include "mutt_menu.h"
#include "muttlib.h"
#include "observer.h"
#include "opcodes.h"
#include "view.h"
#include "wdata.h"

/* These Config Variables are only used in sidebar.c */
short C_SidebarComponentDepth; ///< Config: (sidebar) Strip leading path components from sidebar folders
char *C_SidebarDelimChars; ///< Config: (sidebar) Characters that separate nested folders
char *C_SidebarDividerChar; ///< Config: (sidebar) Character to draw between the sidebar and index
bool C_SidebarFolderIndent; ///< Config: (sidebar) Indent nested folders
char *C_SidebarFormat; ///< Config: (sidebar) printf-like format string for the sidebar panel
char *C_SidebarIndentString; ///< Config: (sidebar) Indent nested folders using this string
bool C_SidebarNewMailOnly; ///< Config: (sidebar) Only show folders with new/flagged mail
bool C_SidebarNonEmptyMailboxOnly; ///< Config: (sidebar) Only show folders with a non-zero number of mail
bool C_SidebarNextNewWrap; ///< Config: (sidebar) Wrap around when searching for the next mailbox with new mail
bool C_SidebarOnRight; ///< Config: (sidebar) Display the sidebar on the right
bool C_SidebarShortPath; ///< Config: (sidebar) Abbreviate the paths using the #C_Folder variable
short C_SidebarSortMethod; ///< Config: (sidebar) Method to sort the sidebar
bool C_SidebarVisible;     ///< Config: (sidebar) Show the sidebar
short C_SidebarWidth;      ///< Config: (sidebar) Width of the sidebar

struct ListHead SidebarWhitelist = STAILQ_HEAD_INITIALIZER(SidebarWhitelist); ///< List of mailboxes to always display in the sidebar

/**
 * struct SbEntry - Info about folders in the sidebar
 */
struct SbEntry
{
  char box[256];           ///< Formatted Mailbox name
  struct Mailbox *mailbox; ///< Mailbox this represents
  bool is_hidden;          ///< Don't show, e.g. $sidebar_new_mail_only
  enum ColorId color;      ///< Colour to use
};

/**
 * sidebar_format_str - Format a string for the sidebar - Implements ::format_t
 *
 * | Expando | Description
 * |:--------|:--------------------------------------------------------
 * | \%!     | 'n!' Flagged messages
 * | \%B     | Name of the mailbox
 * | \%D     | Description of the mailbox
 * | \%d     | Number of deleted messages
 * | \%F     | Number of Flagged messages in the mailbox
 * | \%L     | Number of messages after limiting
 * | \%n     | 'N' if mailbox has new mail, ' ' (space) otherwise
 * | \%N     | Number of unread messages in the mailbox
 * | \%o     | Number of old unread messages in the mailbox
 * | \%r     | Number of read messages in the mailbox
 * | \%S     | Size of mailbox (total number of messages)
 * | \%t     | Number of tagged messages
 * | \%Z     | Number of new unseen messages in the mailbox
 */
static const char *sidebar_format_str(char *buf, size_t buflen, size_t col, int cols,
                                      char op, const char *src, const char *prec,
                                      const char *if_str, const char *else_str,
                                      intptr_t data, MuttFormatFlags flags)
{
  struct SbEntry *sbe = (struct SbEntry *) data;
  char fmt[256];

  if (!sbe || !buf)
    return src;

  buf[0] = '\0'; /* Just in case there's nothing to do */

  struct Mailbox *m = sbe->mailbox;
  if (!m)
    return src;

  bool c = Context && Context->mailbox &&
           (mutt_str_strcmp(Context->mailbox->realpath, m->realpath) == 0);

  bool optional = (flags & MUTT_FORMAT_OPTIONAL);

  switch (op)
  {
    case 'B':
      mutt_format_s(buf, buflen, prec, sbe->box);
      break;

    case 'd':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sd", prec);
        snprintf(buf, buflen, fmt, c ? Context->mailbox->msg_deleted : 0);
      }
      else if ((c && (Context->mailbox->msg_deleted == 0)) || !c)
        optional = false;
      break;

    case 'D':
      if (sbe->mailbox->name)
        mutt_format_s(buf, buflen, prec, sbe->mailbox->name);
      else
        mutt_format_s(buf, buflen, prec, sbe->box);
      break;

    case 'F':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sd", prec);
        snprintf(buf, buflen, fmt, m->msg_flagged);
      }
      else if (m->msg_flagged == 0)
        optional = false;
      break;

    case 'L':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sd", prec);
        snprintf(buf, buflen, fmt, c ? Context->mailbox->vcount : m->msg_count);
      }
      else if ((c && (Context->mailbox->vcount == m->msg_count)) || !c)
        optional = false;
      break;

    case 'N':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sd", prec);
        snprintf(buf, buflen, fmt, m->msg_unread);
      }
      else if (m->msg_unread == 0)
        optional = false;
      break;

    case 'n':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sc", prec);
        snprintf(buf, buflen, fmt, m->has_new ? 'N' : ' ');
      }
      else if (m->has_new == false)
        optional = false;
      break;

    case 'o':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sd", prec);
        snprintf(buf, buflen, fmt, m->msg_unread - m->msg_new);
      }
      else if ((c && (Context->mailbox->msg_unread - Context->mailbox->msg_new) == 0) || !c)
        optional = false;
      break;

    case 'r':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sd", prec);
        snprintf(buf, buflen, fmt, m->msg_count - m->msg_unread);
      }
      else if ((c && (Context->mailbox->msg_count - Context->mailbox->msg_unread) == 0) || !c)
        optional = false;
      break;

    case 'S':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sd", prec);
        snprintf(buf, buflen, fmt, m->msg_count);
      }
      else if (m->msg_count == 0)
        optional = false;
      break;

    case 't':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sd", prec);
        snprintf(buf, buflen, fmt, c ? Context->mailbox->msg_tagged : 0);
      }
      else if ((c && (Context->mailbox->msg_tagged == 0)) || !c)
        optional = false;
      break;

    case 'Z':
      if (!optional)
      {
        snprintf(fmt, sizeof(fmt), "%%%sd", prec);
        snprintf(buf, buflen, fmt, m->msg_new);
      }
      else if ((c && (Context->mailbox->msg_new) == 0) || !c)
        optional = false;
      break;

    case '!':
      if (m->msg_flagged == 0)
        mutt_format_s(buf, buflen, prec, "");
      else if (m->msg_flagged == 1)
        mutt_format_s(buf, buflen, prec, "!");
      else if (m->msg_flagged == 2)
        mutt_format_s(buf, buflen, prec, "!!");
      else
      {
        snprintf(fmt, sizeof(fmt), "%d!", m->msg_flagged);
        mutt_format_s(buf, buflen, prec, fmt);
      }
      break;
  }

  if (optional)
  {
    mutt_expando_format(buf, buflen, col, C_SidebarWidth, if_str,
                        sidebar_format_str, IP sbe, flags);
  }
  else if (flags & MUTT_FORMAT_OPTIONAL)
  {
    mutt_expando_format(buf, buflen, col, C_SidebarWidth, else_str,
                        sidebar_format_str, IP sbe, flags);
  }

  /* We return the format string, unchanged */
  return src;
}

/**
 * make_sidebar_entry - Turn mailbox data into a sidebar string
 * @param[out] buf     Buffer in which to save string
 * @param[in]  buflen  Buffer length
 * @param[in]  width   Desired width in screen cells
 * @param[in]  box     Mailbox name
 * @param[in]  sbe     Mailbox object
 *
 * Take all the relevant mailbox data and the desired screen width and then get
 * mutt_expando_format to do the actual work. mutt_expando_format will callback to
 * us using sidebar_format_str() for the sidebar specific formatting characters.
 */
static void make_sidebar_entry(char *buf, size_t buflen, int width,
                               const char *box, struct SbEntry *sbe)
{
  mutt_str_strfcpy(sbe->box, box, sizeof(sbe->box));

  mutt_expando_format(buf, buflen, 0, width, NONULL(C_SidebarFormat),
                      sidebar_format_str, IP sbe, MUTT_FORMAT_NO_FLAGS);

  /* Force string to be exactly the right width */
  int w = mutt_strwidth(buf);
  int s = mutt_str_strlen(buf);
  width = MIN(buflen, width);
  if (w < width)
  {
    /* Pad with spaces */
    memset(buf + s, ' ', width - w);
    buf[s + width - w] = '\0';
  }
  else if (w > width)
  {
    /* Truncate to fit */
    size_t len = mutt_wstr_trunc(buf, buflen, width, NULL);
    buf[len] = '\0';
  }
}

/**
 * cb_qsort_sbe - qsort callback to sort SbEntry's
 * @param a First  SbEntry to compare
 * @param b Second SbEntry to compare
 * @retval -1 a precedes b
 * @retval  0 a and b are identical
 * @retval  1 b precedes a
 */
static int cb_qsort_sbe(const void *a, const void *b)
{
  const struct SbEntry *sbe1 = *(struct SbEntry const *const *) a;
  const struct SbEntry *sbe2 = *(struct SbEntry const *const *) b;
  const struct Mailbox *m1 = sbe1->mailbox;
  const struct Mailbox *m2 = sbe2->mailbox;

  int rc = 0;

  switch ((C_SidebarSortMethod & SORT_MASK))
  {
    case SORT_COUNT:
      if (m2->msg_count == m1->msg_count)
        rc = mutt_str_strcoll(mailbox_path(m1), mailbox_path(m2));
      else
        rc = (m2->msg_count - m1->msg_count);
      break;
    case SORT_UNREAD:
      if (m2->msg_unread == m1->msg_unread)
        rc = mutt_str_strcoll(mailbox_path(m1), mailbox_path(m2));
      else
        rc = (m2->msg_unread - m1->msg_unread);
      break;
    case SORT_DESC:
      rc = mutt_str_strcmp(m1->name, m2->name);
      break;
    case SORT_FLAGGED:
      if (m2->msg_flagged == m1->msg_flagged)
        rc = mutt_str_strcoll(mailbox_path(m1), mailbox_path(m2));
      else
        rc = (m2->msg_flagged - m1->msg_flagged);
      break;
    case SORT_PATH:
    {
      rc = mutt_inbox_cmp(mailbox_path(m1), mailbox_path(m2));
      if (rc == 0)
        rc = mutt_str_strcoll(mailbox_path(m1), mailbox_path(m2));
      break;
    }
  }

  if (C_SidebarSortMethod & SORT_REVERSE)
    rc = -rc;

  return rc;
}

/**
 * update_entries_visibility - Should a SbEntry be displayed in the sidebar?
 *
 * For each SbEntry in the entries array, check whether we should display it.
 * This is determined by several criteria.  If the Mailbox:
 * * is the currently open mailbox
 * * is the currently highlighted mailbox
 * * has unread messages
 * * has flagged messages
 * * is whitelisted
 */
static void update_entries_visibility(struct SidebarWindowData *wdata)
{
  /* Aliases for readability */
  const bool new_only = C_SidebarNewMailOnly;
  const bool non_empty_only = C_SidebarNonEmptyMailboxOnly;
  struct SbEntry *sbe = NULL;

  /* Take the fast path if there is no need to test visibilities */
  if (!new_only && !non_empty_only)
  {
    for (int i = 0; i < wdata->entry_count; i++)
    {
      wdata->entries[i]->is_hidden = false;
    }
    return;
  }

  for (int i = 0; i < wdata->entry_count; i++)
  {
    sbe = wdata->entries[i];

    sbe->is_hidden = false;

    if (Context && (mutt_str_strcmp(sbe->mailbox->realpath, Context->mailbox->realpath) == 0))
    {
      /* Spool directories are always visible */
      continue;
    }

    if (mutt_list_find(&SidebarWhitelist, mailbox_path(sbe->mailbox)) ||
        mutt_list_find(&SidebarWhitelist, sbe->mailbox->name))
    {
      /* Explicitly asked to be visible */
      continue;
    }

    if (non_empty_only && (i != wdata->opn_index) && (sbe->mailbox->msg_count == 0))
    {
      sbe->is_hidden = true;
    }

    if (new_only && (i != wdata->opn_index) && (sbe->mailbox->msg_unread == 0) &&
        (sbe->mailbox->msg_flagged == 0) && !sbe->mailbox->has_new)
    {
      sbe->is_hidden = true;
    }
  }
}

/**
 * unsort_entries - Restore wdata->entries array order to match Mailbox list order
 */
static void unsort_entries(struct SidebarWindowData *wdata)
{
  int i = 0;

  struct MailboxList ml = neomutt_mailboxlist_get_all(NeoMutt, MUTT_MAILBOX_ANY);
  struct MailboxNode *np = NULL;
  STAILQ_FOREACH(np, &ml, entries)
  {
    if (i >= wdata->entry_count)
      break;

    int j = i;
    while ((j < wdata->entry_count) && (wdata->entries[j]->mailbox != np->mailbox))
      j++;
    if (j < wdata->entry_count)
    {
      if (j != i)
      {
        struct SbEntry *tmp = wdata->entries[i];
        wdata->entries[i] = wdata->entries[j];
        wdata->entries[j] = tmp;
      }
      i++;
    }
  }
  neomutt_mailboxlist_clear(&ml);
}

/**
 * sort_entries - Sort wdata->entries array
 *
 * Sort the wdata->entries array according to the current sort config
 * option "sidebar_sort_method". This calls qsort to do the work which calls our
 * callback function "cb_qsort_sbe".
 *
 * Once sorted, the prev/next links will be reconstructed.
 */
static void sort_entries(struct SidebarWindowData *wdata)
{
  enum SortType ssm = (C_SidebarSortMethod & SORT_MASK);

  /* These are the only sort methods we understand */
  if ((ssm == SORT_COUNT) || (ssm == SORT_UNREAD) || (ssm == SORT_FLAGGED) || (ssm == SORT_PATH))
    qsort(wdata->entries, wdata->entry_count, sizeof(*wdata->entries), cb_qsort_sbe);
  else if ((ssm == SORT_ORDER) && (C_SidebarSortMethod != wdata->previous_sort))
    unsort_entries(wdata);
}

/**
 * select_next - Selects the next unhidden mailbox
 * @retval true  Success
 * @retval false Failure
 */
static bool select_next(struct SidebarWindowData *wdata)
{
  if ((wdata->entry_count == 0) || (wdata->hil_index < 0))
    return false;

  int entry = wdata->hil_index;

  do
  {
    entry++;
    if (entry == wdata->entry_count)
      return false;
  } while (wdata->entries[entry]->is_hidden);

  wdata->hil_index = entry;
  return true;
}

/**
 * select_next_new - Selects the next new mailbox
 * @retval true  Success
 * @retval false Failure
 *
 * Search down the list of mail folders for one containing new mail.
 */
static bool select_next_new(struct SidebarWindowData *wdata)
{
  if ((wdata->entry_count == 0) || (wdata->hil_index < 0))
    return false;

  int entry = wdata->hil_index;

  do
  {
    entry++;
    if (entry == wdata->entry_count)
    {
      if (C_SidebarNextNewWrap)
        entry = 0;
      else
        return false;
    }
    if (entry == wdata->hil_index)
      return false;
  } while (!wdata->entries[entry]->mailbox->has_new &&
           (wdata->entries[entry]->mailbox->msg_unread == 0));

  wdata->hil_index = entry;
  return true;
}

/**
 * select_prev - Selects the previous unhidden mailbox
 * @retval true  Success
 * @retval false Failure
 */
static bool select_prev(struct SidebarWindowData *wdata)
{
  if ((wdata->entry_count == 0) || (wdata->hil_index < 0))
    return false;

  int entry = wdata->hil_index;

  do
  {
    entry--;
    if (entry < 0)
      return false;
  } while (wdata->entries[entry]->is_hidden);

  wdata->hil_index = entry;
  return true;
}

/**
 * select_prev_new - Selects the previous new mailbox
 * @retval true  Success
 * @retval false Failure
 *
 * Search up the list of mail folders for one containing new mail.
 */
static bool select_prev_new(struct SidebarWindowData *wdata)
{
  if ((wdata->entry_count == 0) || (wdata->hil_index < 0))
    return false;

  int entry = wdata->hil_index;

  do
  {
    entry--;
    if (entry < 0)
    {
      if (C_SidebarNextNewWrap)
        entry = wdata->entry_count - 1;
      else
        return false;
    }
    if (entry == wdata->hil_index)
      return false;
  } while (!wdata->entries[entry]->mailbox->has_new &&
           (wdata->entries[entry]->mailbox->msg_unread == 0));

  wdata->hil_index = entry;
  return true;
}

/**
 * select_page_down - Selects the first entry in the next page of mailboxes
 * @retval true  Success
 * @retval false Failure
 */
static bool select_page_down(struct SidebarWindowData *wdata)
{
  if ((wdata->entry_count == 0) || (wdata->bot_index < 0))
    return false;

  int orig_hil_index = wdata->hil_index;

  wdata->hil_index = wdata->bot_index;
  select_next(wdata);
  /* If the rest of the entries are hidden, go up to the last unhidden one */
  if (wdata->entries[wdata->hil_index]->is_hidden)
    select_prev(wdata);

  return (orig_hil_index != wdata->hil_index);
}

/**
 * select_page_up - Selects the last entry in the previous page of mailboxes
 * @retval true  Success
 * @retval false Failure
 */
static bool select_page_up(struct SidebarWindowData *wdata)
{
  if ((wdata->entry_count == 0) || (wdata->top_index < 0))
    return false;

  int orig_hil_index = wdata->hil_index;

  wdata->hil_index = wdata->top_index;
  select_prev(wdata);
  /* If the rest of the entries are hidden, go down to the last unhidden one */
  if (wdata->entries[wdata->hil_index]->is_hidden)
    select_next(wdata);

  return (orig_hil_index != wdata->hil_index);
}

/**
 * select_first - Selects the first unhidden mailbox
 * @retval true  Success
 * @retval false Failure
 */
static bool select_first(struct SidebarWindowData *wdata)
{
  if ((wdata->entry_count == 0) || (wdata->hil_index < 0))
    return false;

  int orig_hil_index = wdata->hil_index;

  wdata->hil_index = 0;
  if (wdata->entries[wdata->hil_index]->is_hidden)
    if (!select_next(wdata))
      wdata->hil_index = orig_hil_index;

  return (orig_hil_index != wdata->hil_index);
}

/**
 * select_last - Selects the last unhidden mailbox
 * @retval true  Success
 * @retval false Failure
 */
static bool select_last(struct SidebarWindowData *wdata)
{
  if ((wdata->entry_count == 0) || (wdata->hil_index < 0))
    return false;

  int orig_hil_index = wdata->hil_index;

  wdata->hil_index = wdata->entry_count;
  if (!select_prev(wdata))
    wdata->hil_index = orig_hil_index;

  return (orig_hil_index != wdata->hil_index);
}

/**
 * calc_page - Prepare the list of SbEntry's for the sidebar display
 * @param page_size  The number of lines on a page
 *
 * Before painting the sidebar, we determine which are visible, sort
 * them and set up our page pointers.
 *
 * This is a lot of work to do each refresh, but there are many things that
 * can change outside of the sidebar that we don't hear about.
 */
static void calc_page(struct MuttWindow *win, struct SidebarWindowData *wdata, int page_size)
{
  if (page_size <= 0)
    return;

  const struct SbEntry *opn_entry =
      (wdata->opn_index >= 0) ? wdata->entries[wdata->opn_index] : NULL;
  const struct SbEntry *hil_entry =
      (wdata->hil_index >= 0) ? wdata->entries[wdata->hil_index] : NULL;

  update_entries_visibility(wdata);
  sort_entries(wdata);

  for (int i = 0; i < wdata->entry_count; i++)
  {
    if (opn_entry == wdata->entries[i])
      wdata->opn_index = i;
    if (hil_entry == wdata->entries[i])
      wdata->hil_index = i;
  }

  if ((wdata->hil_index < 0) || wdata->entries[wdata->hil_index]->is_hidden ||
      (C_SidebarSortMethod != wdata->previous_sort))
  {
    if (wdata->opn_index >= 0)
      wdata->hil_index = wdata->opn_index;
    else
    {
      wdata->hil_index = 0;
      if (wdata->entries[wdata->hil_index]->is_hidden)
        select_next(wdata);
    }
  }

  /* Set the Top and Bottom to frame the wdata->hil_index in groups of page_size */

  /* If C_SidebarNewMailOnly or C_SidebarNonEmptyMailboxOnly is set, some entries
   * may be hidden so we need to scan for the framing interval */
  if (C_SidebarNewMailOnly || C_SidebarNonEmptyMailboxOnly)
  {
    wdata->top_index = -1;
    wdata->bot_index = -1;
    while (wdata->bot_index < wdata->hil_index)
    {
      wdata->top_index = wdata->bot_index + 1;
      int page_entries = 0;
      while (page_entries < page_size)
      {
        wdata->bot_index++;
        if (wdata->bot_index >= wdata->entry_count)
          break;
        if (!wdata->entries[wdata->bot_index]->is_hidden)
          page_entries++;
      }
    }
  }
  /* Otherwise we can just calculate the interval */
  else
  {
    wdata->top_index = (wdata->hil_index / page_size) * page_size;
    wdata->bot_index = wdata->top_index + page_size - 1;
  }

  if (wdata->bot_index > (wdata->entry_count - 1))
    wdata->bot_index = wdata->entry_count - 1;

  wdata->previous_sort = C_SidebarSortMethod;
  return;
}

/**
 * calc_divider - Calculate the space needed for the divider
 * @param wdata Sidebar data
 */
static void calc_divider(struct SidebarWindowData *wdata)
{
  wdata->divider_type = SB_DIV_UTF8;

  /* Calculate the width of the delimiter in screen cells */
  wdata->divider_width = mutt_strwidth(C_SidebarDividerChar);
  if (wdata->divider_width < 0)
  {
    wdata->divider_width = 1; /* Bad character */
  }
  else if (wdata->divider_width == 0)
  {
    if (C_SidebarDividerChar)
      return; /* User has set empty string */

    wdata->divider_width = 1; /* Unset variable */
  }
  else
  {
    wdata->divider_type = SB_DIV_USER; /* User config */
  }

  if (C_AsciiChars && (wdata->divider_type != SB_DIV_ASCII))
  {
    /* $ascii_chars overrides Unicode divider chars */
    if (wdata->divider_type == SB_DIV_UTF8)
    {
      wdata->divider_type = SB_DIV_ASCII;
    }
    else if (C_SidebarDividerChar)
    {
      for (int i = 0; i < wdata->divider_width; i++)
      {
        if (C_SidebarDividerChar[i] & ~0x7F) /* high-bit is set */
        {
          wdata->divider_type = SB_DIV_ASCII;
          wdata->divider_width = 1;
          break;
        }
      }
    }
  }
}

/**
 * draw_divider - Draw a line between the sidebar and the rest of neomutt
 * @param win Window to draw on
 *
 * Draw a divider using characters from the config option "sidebar_divider_char".
 * This can be an ASCII or Unicode character.
 * We calculate these characters' width in screen columns.
 *
 * If the user hasn't set $sidebar_divider_char we pick a character for them,
 * respecting the value of $ascii_chars.
 */
static void draw_divider(struct SidebarWindowData *wdata, struct MuttWindow *win)
{
  int num_rows = win->state.rows;
  int num_cols = win->state.cols;

  if ((num_rows < 1) || (num_cols < 1) || (wdata->divider_width > num_cols))
    return;

  mutt_curses_set_color(MT_COLOR_SIDEBAR_DIVIDER);

  int col = C_SidebarOnRight ? 0 : (C_SidebarWidth - wdata->divider_width);

  for (int i = 0; i < num_rows; i++)
  {
    mutt_window_move(win, col, i);

    switch (wdata->divider_type)
    {
      case SB_DIV_USER:
        mutt_window_addstr(C_SidebarDividerChar);
        break;
      case SB_DIV_ASCII:
        mutt_window_addch('|');
        break;
      case SB_DIV_UTF8:
        mutt_window_addch(ACS_VLINE);
        break;
    }
  }
}

/**
 * fill_empty_space - Wipe the remaining Sidebar space
 * @param win        Window to draw on
 * @param first_row  Window line to start (0-based)
 * @param num_rows   Number of rows to fill
 * @param div_width  Width in screen characters taken by the divider
 * @param num_cols   Number of columns to fill
 *
 * Write spaces over the area the sidebar isn't using.
 */
static void fill_empty_space(struct MuttWindow *win, int first_row,
                             int num_rows, int div_width, int num_cols)
{
  /* Fill the remaining rows with blank space */
  mutt_curses_set_color(MT_COLOR_NORMAL);

  if (!C_SidebarOnRight)
    div_width = 0;
  for (int r = 0; r < num_rows; r++)
  {
    mutt_window_move(win, div_width, first_row + r);

    for (int i = 0; i < num_cols; i++)
      mutt_window_addch(' ');
  }
}

/**
 * imap_is_prefix - Check if folder matches the beginning of mbox
 * @param folder Folder
 * @param mbox   Mailbox path
 * @retval num Length of the prefix
 */
static int imap_is_prefix(const char *folder, const char *mbox)
{
  int plen = 0;

  struct Url *url_m = url_parse(mbox);
  struct Url *url_f = url_parse(folder);
  if (!url_m || !url_f)
    goto done;

  if (mutt_str_strcasecmp(url_m->host, url_f->host) != 0)
    goto done;

  if (url_m->user && url_f->user && (mutt_str_strcasecmp(url_m->user, url_f->user) != 0))
    goto done;

  size_t mlen = mutt_str_strlen(url_m->path);
  size_t flen = mutt_str_strlen(url_f->path);
  if (flen > mlen)
    goto done;

  if (mutt_str_strncmp(url_m->path, url_f->path, flen) != 0)
    goto done;

  plen = strlen(mbox) - mlen + flen;

done:
  url_free(&url_m);
  url_free(&url_f);

  return plen;
}

/**
 * abbrev_folder - Abbreviate a Mailbox path using a folder
 * @param mbox   Mailbox path to shorten
 * @param folder Folder path to use
 * @param type   Mailbox type
 * @retval ptr Pointer into the mbox param
 */
static const char *abbrev_folder(const char *mbox, const char *folder, enum MailboxType type)
{
  if (!mbox || !folder)
    return NULL;

  if (type == MUTT_IMAP)
  {
    int prefix = imap_is_prefix(folder, mbox);
    if (prefix == 0)
      return NULL;
    return mbox + prefix;
  }

  if (!C_SidebarDelimChars)
    return NULL;

  size_t flen = mutt_str_strlen(folder);
  if (flen == 0)
    return NULL;
  if (strchr(C_SidebarDelimChars, folder[flen - 1])) // folder ends with a delimiter
    flen--;

  size_t mlen = mutt_str_strlen(mbox);
  if (mlen <= flen)
    return NULL;

  if (mutt_str_strncmp(folder, mbox, flen) != 0)
    return NULL;

  // After the match, check that mbox has a delimiter
  if (!strchr(C_SidebarDelimChars, mbox[flen]))
    return NULL;

  return mbox + flen + 1;
}

/**
 * abbrev_url - Abbreviate a url-style Mailbox path
 * @param mbox Mailbox path to shorten
 * @param type Mailbox type
 *
 * Use heuristics to shorten a non-local Mailbox path.
 * Strip the host part (or database part for Notmuch).
 *
 * e.g.
 * - `imap://user@host.com/apple/banana` becomes `apple/banana`
 * - `notmuch:///home/user/db?query=hello` becomes `query=hello`
 */
static const char *abbrev_url(const char *mbox, enum MailboxType type)
{
  /* This is large enough to skip `notmuch://`,
   * but not so large that it will go past the host part. */
  const int scheme_len = 10;

  size_t len = mutt_str_strlen(mbox);
  if ((len < scheme_len) || ((type != MUTT_NNTP) && (type != MUTT_IMAP) &&
                             (type != MUTT_NOTMUCH) && (type != MUTT_POP)))
  {
    return mbox;
  }

  const char split = (type == MUTT_NOTMUCH) ? '?' : '/';

  // Skip over the scheme, e.g. `imaps://`, `notmuch://`
  const char *last = strchr(mbox + scheme_len, split);
  if (last)
    mbox = last + 1;
  return mbox;
}

/**
 * calc_path_depth - Calculate the depth of a Mailbox path
 * @param[in]  mbox      Mailbox path to examine
 * @param[in]  delims    Delimiter characters
 * @param[out] last_part Last path component
 */
static int calc_path_depth(const char *mbox, const char *delims, const char **last_part)
{
  if (!mbox || !delims || !last_part)
    return 0;

  int depth = 0;
  const char *match = NULL;
  while ((match = strpbrk(mbox, delims)))
  {
    depth++;
    mbox = match + 1;
  }

  *last_part = mbox;
  return depth;
}

/**
 * calc_entries - Write out a list of mailboxes, in a panel
 * @param win Window to draw on
 *
 * Display a list of mailboxes in a panel on the left.  What's displayed will
 * depend on our index markers: TopMailbox, OpnMailbox, HilMailbox, BotMailbox.
 * On the first run they'll be NULL, so we display the top of NeoMutt's list.
 *
 * * TopMailbox - first visible mailbox
 * * BotMailbox - last  visible mailbox
 * * OpnMailbox - mailbox shown in NeoMutt's Index Panel
 * * HilMailbox - Unselected mailbox (the paging follows this)
 *
 * The entries are formatted using "sidebar_format" and may be abbreviated:
 * "sidebar_short_path", indented: "sidebar_folder_indent",
 * "sidebar_indent_string" and sorted: "sidebar_sort_method".  Finally, they're
 * trimmed to fit the available space.
 */
static void calc_entries(struct MuttWindow *win)
{
  int num_rows = win->state.rows;
  int num_cols = win->state.cols;

  struct SbEntry *entry = NULL;
  struct Mailbox *m = NULL;
  struct SidebarWindowData *wdata = sb_wdata_get(win);
  if (wdata->top_index < 0)
    return;

  int w = MIN(num_cols, (C_SidebarWidth - wdata->divider_width));
  int row = 0;
  const char *display = NULL;
  struct Buffer result = mutt_buffer_make(256);
  for (int entryidx = wdata->top_index;
       (entryidx < wdata->entry_count) && (row < num_rows); entryidx++)
  {
    entry = wdata->entries[entryidx];
    if (entry->is_hidden)
      continue;
    m = entry->mailbox;

    if (entryidx == wdata->opn_index)
    {
      if ((Colors->defs[MT_COLOR_SIDEBAR_INDICATOR] != 0))
        entry->color = MT_COLOR_SIDEBAR_INDICATOR;
      else
        entry->color = MT_COLOR_INDICATOR;
    }
    else if (entryidx == wdata->hil_index)
      entry->color = MT_COLOR_SIDEBAR_HIGHLIGHT;
    else if (m->has_new)
      entry->color = MT_COLOR_SIDEBAR_NEW;
    else if (m->msg_unread > 0)
      entry->color = MT_COLOR_SIDEBAR_UNREAD;
    else if (m->msg_flagged > 0)
      entry->color = MT_COLOR_SIDEBAR_FLAGGED;
    else if ((Colors->defs[MT_COLOR_SIDEBAR_SPOOLFILE] != 0) &&
             (mutt_str_strcmp(mailbox_path(m), C_Spoolfile) == 0))
    {
      entry->color = MT_COLOR_SIDEBAR_SPOOLFILE;
    }
    else
    {
      if (Colors->defs[MT_COLOR_SIDEBAR_ORDINARY] != 0)
        entry->color = MT_COLOR_SIDEBAR_ORDINARY;
      else
        entry->color = MT_COLOR_NORMAL;
    }

    if (Context && Context->mailbox && (Context->mailbox->realpath[0] != '\0') &&
        (mutt_str_strcmp(m->realpath, Context->mailbox->realpath) == 0))
    {
      m->msg_unread = Context->mailbox->msg_unread;
      m->msg_count = Context->mailbox->msg_count;
      m->msg_flagged = Context->mailbox->msg_flagged;
    }

    const char *full_path = mailbox_path(m);
    display = m->name;
    if (!display)
      display = full_path;

    const char *abbr = m->name;
    if (!abbr)
      abbr = abbrev_folder(display, C_Folder, m->type);
    if (!abbr)
      abbr = abbrev_url(display, m->type);

    // Use the abbreviation if we have one. The full path is not preferable.
    if (abbr)
      display = abbr;

    const char *last_part = abbr;
    int depth = calc_path_depth(abbr, C_SidebarDelimChars, &last_part);

    // At this point, we don't have an abbreviation so let's keep track
    // before using short path.
    bool no_abbr = (mutt_str_strncmp(display, full_path, mutt_str_strlen(display)) != 0);
    if (C_SidebarShortPath)
      display = last_part;

    mutt_buffer_reset(&result);

    // Don't indent if we were unable to create an abbreviation.
    // Otherwise, the full path will be indent, and it looks unusual.
    if (C_SidebarFolderIndent && no_abbr)
    {
      if (C_SidebarComponentDepth > 0)
        depth -= C_SidebarComponentDepth;

      for (int i = 0; i < depth; i++)
        mutt_buffer_addstr(&result, C_SidebarIndentString);
    }

    mutt_buffer_addstr(&result, display);

    char str[256];
    make_sidebar_entry(str, sizeof(str), w, mutt_b2s(&result), entry);
    mutt_str_strfcpy(entry->box, str, sizeof(entry->box));
    row++;
  }
  mutt_buffer_dealloc(&result);
}

/**
 * draw_data - Draw the sidebar data onto the Window
 * @param win Window to draw on
 */
static void draw_data(struct MuttWindow *win)
{
  int num_rows = win->state.rows;
  int num_cols = win->state.cols;
  struct SbEntry *entry = NULL;
  struct SidebarWindowData *wdata = sb_wdata_get(win);
  int w = MIN(num_cols, (C_SidebarWidth - wdata->divider_width));
  int row = 0;
  int col = C_SidebarOnRight ? wdata->divider_width : 0;

  fill_empty_space(win, row, num_rows - row, wdata->divider_width, w);
  for (int entryidx = wdata->top_index;
       (entryidx < wdata->entry_count) && (row < num_rows); entryidx++)
  {
    entry = wdata->entries[entryidx];
    if (entry->is_hidden)
      continue;

    mutt_curses_set_color(entry->color);
    mutt_window_mvaddstr(win, col, row, entry->box);
    row++;
  }

  fill_empty_space(win, row, num_rows - row, wdata->divider_width, w);
}

/**
 * sb_change_mailbox - XXX
 */
void sb_change_mailbox(struct MuttWindow *win, int op)
{
  if (!C_SidebarVisible)
    return;

  struct SidebarWindowData *wdata = sb_wdata_get(win);
  if (!wdata)
    return;

  if (wdata->hil_index < 0) /* It'll get reset on the next draw */
    return;

  switch (op)
  {
    case OP_SIDEBAR_FIRST:
      if (!select_first(wdata))
        return;
      break;
    case OP_SIDEBAR_LAST:
      if (!select_last(wdata))
        return;
      break;
    case OP_SIDEBAR_NEXT:
      if (!select_next(wdata))
        return;
      break;
    case OP_SIDEBAR_NEXT_NEW:
      if (!select_next_new(wdata))
        return;
      break;
    case OP_SIDEBAR_PAGE_DOWN:
      if (!select_page_down(wdata))
        return;
      break;
    case OP_SIDEBAR_PAGE_UP:
      if (!select_page_up(wdata))
        return;
      break;
    case OP_SIDEBAR_PREV:
      if (!select_prev(wdata))
        return;
      break;
    case OP_SIDEBAR_PREV_NEW:
      if (!select_prev_new(wdata))
        return;
      break;
    default:
      return;
  }
  mutt_menu_set_current_redraw(REDRAW_SIDEBAR);
}

/**
 * sb_get_highlight - Get the Mailbox that's highlighted in the sidebar
 * @retval ptr Mailbox
 */
struct Mailbox *sb_get_highlight(struct MuttWindow *win)
{
  if (!C_SidebarVisible)
    return NULL;

  struct SidebarWindowData *wdata = sb_wdata_get(win);
  if ((wdata->entry_count == 0) || (wdata->hil_index < 0))
    return NULL;

  return wdata->entries[wdata->hil_index]->mailbox;
}

/**
 * sb_notify_mailbox - The state of a Mailbox is about to change
 * @param m       Folder
 * @param created True if folder created, false if deleted
 *
 * We receive a notification:
 * - After a new Mailbox has been created
 * - Before a Mailbox is deleted
 *
 * Before a deletion, check that our pointers won't be invalidated.
 */
void sb_notify_mailbox(struct Mailbox *m, bool created)
{
  if (!m)
    return;

  struct MuttWindow *win = NULL;
  struct SidebarWindowData *wdata = sb_wdata_get(win);

  /* Any new/deleted mailboxes will cause a refresh.  As long as
   * they're valid, our pointers will be updated in calc_page() */

  if (created)
  {
    if (wdata->entry_count >= wdata->entry_max)
    {
      wdata->entry_max += 10;
      mutt_mem_realloc(&wdata->entries, wdata->entry_max * sizeof(struct SbEntry *));
    }
    wdata->entries[wdata->entry_count] = mutt_mem_calloc(1, sizeof(struct SbEntry));
    wdata->entries[wdata->entry_count]->mailbox = m;

    if (wdata->top_index < 0)
      wdata->top_index = wdata->entry_count;
    if (wdata->bot_index < 0)
      wdata->bot_index = wdata->entry_count;
    if ((wdata->opn_index < 0) && Context &&
        (mutt_str_strcmp(m->realpath, Context->mailbox->realpath) == 0))
    {
      wdata->opn_index = wdata->entry_count;
    }

    wdata->entry_count++;
  }
  else
  {
    int del_index;
    for (del_index = 0; del_index < wdata->entry_count; del_index++)
      if (wdata->entries[del_index]->mailbox == m)
        break;
    if (del_index == wdata->entry_count)
      return;
    FREE(&wdata->entries[del_index]);
    wdata->entry_count--;

    if ((wdata->top_index > del_index) || (wdata->top_index == wdata->entry_count))
      wdata->top_index--;
    if (wdata->opn_index == del_index)
      wdata->opn_index = -1;
    else if (wdata->opn_index > del_index)
      wdata->opn_index--;
    if ((wdata->hil_index > del_index) || (wdata->hil_index == wdata->entry_count))
      wdata->hil_index--;
    if ((wdata->bot_index > del_index) || (wdata->bot_index == wdata->entry_count))
      wdata->bot_index--;

    for (; del_index < wdata->entry_count; del_index++)
      wdata->entries[del_index] = wdata->entries[del_index + 1];
  }

  mutt_menu_set_current_redraw(REDRAW_SIDEBAR);
}

/**
 * init_data - Initialise the sidebar data
 *
 * Get a list of all the Mailboxes.
 */
static void init_data(void)
{
  struct MailboxList ml = neomutt_mailboxlist_get_all(NeoMutt, MUTT_MAILBOX_ANY);
  struct MailboxNode *np = NULL;
  STAILQ_FOREACH(np, &ml, entries)
  {
    sb_notify_mailbox(np->mailbox, true);
  }
  neomutt_mailboxlist_clear(&ml);
}

/**
 * sb_recalc - XXX
 */
int sb_recalc(void)
{
  mutt_debug(LL_DEBUG1, "SIDEBAR RECALC\n");
  return -1;
}

/**
 * sb_repaint - XXX
 */
int sb_repaint(void)
{
  mutt_debug(LL_DEBUG1, "SIDEBAR REPAINT\n");
  return -1;
}

/**
 * sb_draw - Completely redraw the sidebar
 * @param win Window to draw on
 *
 * Completely refresh the sidebar region.  First calculate everything to be
 * displayed, then display it and finally blank out any remaining space.
 */
void sb_draw(struct MuttWindow *win)
{
  if (!mutt_window_is_visible(win))
    return;

  struct SidebarWindowData *wdata = sb_wdata_get(win);
  if (!wdata)
    return;

  if (!wdata->entries)
    init_data(/*wdata*/);

  calc_divider(wdata);
  calc_page(win, wdata, win->state.rows);
  calc_entries(win);

  draw_divider(wdata, win);
  draw_data(win);
}

/**
 * sb_win_init - XXX
 */
void sb_win_init(struct MuttWindow *dlg)
{
  dlg->orient = MUTT_WIN_ORIENT_HORIZONTAL;

  struct MuttWindow *index_panel = TAILQ_FIRST(&dlg->children);
  TAILQ_REMOVE(&dlg->children, index_panel, entries);

  struct MuttWindow *pager_panel = TAILQ_FIRST(&dlg->children);
  TAILQ_REMOVE(&dlg->children, pager_panel, entries);

  struct MuttWindow *cont_right =
      mutt_window_new(WT_CONTAINER, MUTT_WIN_ORIENT_VERTICAL, MUTT_WIN_SIZE_MAXIMISE,
                      MUTT_WIN_SIZE_UNLIMITED, MUTT_WIN_SIZE_UNLIMITED);

  mutt_window_add_child(cont_right, index_panel);
  mutt_window_add_child(cont_right, pager_panel);

  struct MuttWindow *win_sidebar =
      mutt_window_new(WT_SIDEBAR, MUTT_WIN_ORIENT_HORIZONTAL, MUTT_WIN_SIZE_FIXED,
                      C_SidebarWidth, MUTT_WIN_SIZE_UNLIMITED);
  win_sidebar->state.visible = C_SidebarVisible && (C_SidebarWidth > 0);
  win_sidebar->wdata = sb_wdata_new();
  win_sidebar->wdata_free = sb_wdata_free;

  if (C_SidebarOnRight)
  {
    mutt_window_add_child(dlg, cont_right);
    mutt_window_add_child(dlg, win_sidebar);
  }
  else
  {
    mutt_window_add_child(dlg, win_sidebar);
    mutt_window_add_child(dlg, cont_right);
  }

  win_sidebar->wdata = sb_windata_new();
  win_sidebar->wdata_free = sb_windata_free;

  sb_windata_populate(win_sidebar->wdata);

  // Only listen to OUR index events
  notify_observer_add(dlg->notify, sb_dialog_observer, win_sidebar);
  // Listen to global events
  notify_observer_add(NeoMutt->notify, sb_neomutt_observer, win_sidebar);
}

/**
 * sb_init - Set up the Sidebar
 */
void sb_init(void)
{
  // Soon this will initialise the Sidebar's:
  // - Colours
  // - Commands
  // - Config
  // - Functions
}

/**
 * sb_shutdown - Clean up the Sidebar
 */
void sb_shutdown(void)
{
  mutt_list_free(&SidebarWhitelist);
}
