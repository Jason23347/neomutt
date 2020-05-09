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

#ifndef MUTT_SIDEBAR_WDATA_H
#define MUTT_SIDEBAR_WDATA_H

#include "sidebar/lib.h"

/**
 * struct SidebarWindowData - Sidebar private Window data - @extends MuttWindow
 */
struct SidebarWindowData
{
  struct SbEntry **entries; ///< Items to display in the sidebar
  int entry_count;          ///< Number of items in entries
  int entry_max;            ///< Size of the entries array

  int top_index; ///< First mailbox visible in sidebar
  int opn_index; ///< Current (open) mailbox
  int hil_index; ///< Highlighted mailbox
  int bot_index; ///< Last mailbox visible in sidebar

  short previous_sort; ///< sidebar_sort_method

  short divider_width;       ///< XXX
  enum DivType divider_type; ///< XXX
};

void                      sb_wdata_free(struct MuttWindow *win, void **ptr);
struct SidebarWindowData *sb_wdata_new(void);
struct SidebarWindowData *sb_wdata_get(struct MuttWindow *win);

void                   sb_windata_free(struct MuttWindow *win, void **ptr);
struct SidebarWinData *sb_windata_new(void);
void                   sb_windata_populate(struct SidebarWinData *data);

#endif /* MUTT_SIDEBAR_WDATA_H */
