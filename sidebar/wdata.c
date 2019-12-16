/**
 * @file
 * Sidebar Window data
 *
 * @authors
 * Copyright (C) 2020 Richard Russon <rich@flatcap.org>
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
 * @page sidebar_wdata Sidebar Window data
 *
 * Sidebar Window data
 */

#include "config.h"
#include "mutt/lib.h"
#include "core/lib.h"
#include "sidebar/wdata.h"
#include "sidebar/view.h"

struct MuttWindow;

/**
 * sb_wdata_new - XXX
 */
struct SidebarWindowData *sb_wdata_new(void)
{
  return mutt_mem_calloc(1, sizeof(struct SidebarWindowData));
}

/**
 * sb_wdata_free - XXX
 */
void sb_wdata_free(struct MuttWindow *win, void **ptr)
{
  if (!ptr || !*ptr)
    return;

  // struct SidebarWindowData *sd = *ptr;

  FREE(ptr);
}

/**
 * sb_windata_free - XXX
 */
void sb_windata_free(struct MuttWindow *win, void **ptr)
{
  struct SidebarWinData *data = *ptr;

  // notify_observer_remove(NeoMutt->notify, sb_observer, win);

  for (size_t i = 0; i < data->num_accounts; i++)
  {
    sb_account_free(&data->accounts[i]);
  }

  FREE(&data->accounts);
  FREE(ptr);
}

/**
 * sb_windata_new - XXX
 */
struct SidebarWinData *sb_windata_new(void)
{
  struct SidebarWinData *data = mutt_mem_calloc(1, sizeof(*data));

  data->max_accounts = 25;
  data->accounts =
      mutt_mem_calloc(data->max_accounts, sizeof(struct SidebarAccountView *));

  return data;
}

/**
 * sb_windata_populate - XXX
 */
void sb_windata_populate(struct SidebarWinData *data)
{
  struct Account *np = NULL;

  TAILQ_FOREACH(np, &NeoMutt->accounts, entries)
  {
    sb_account_add(data, np);
  }

  sb_account_dump(data);
}
