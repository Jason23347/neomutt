/**
 * @file
 * Sidebar observers
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
 * @page sidebar_observers Sidebar observers
 *
 * Sidebar observers
 */

#include "config.h"
#include "mutt/lib.h"
#include "core/lib.h"
#include "gui/lib.h"
#include "index.h"

/**
 * sb_dialog_observer - XXX
 */
int sb_dialog_observer(struct NotifyCallback *nc)
{
  if (!nc || !nc->event_data || !nc->global_data)
    return -1;
  if (nc->event_type != NT_WINDOW)
    return 0;

  // struct MuttWindow *win = nc->global_data;
  // struct SidebarWindowData *sb_data = win->wdata;
  // struct EventIndexDlg *eid = nc->event_data;

  switch (nc->event_subtype)
  {
    // case NT_WINDOW_NEW:
    // case NT_WINDOW_DELETE:
    // case NT_WINDOW_STATE:
    // case NT_INDEX_MAILBOX:
  }

  // if (!nc->event_data || !nc->global_data)
  //   return -1;
  // if (nc->event_type != NT_WINDOW)
  //   return 0;
  // if (nc->event_subtype != NT_WINDOW_DELETE)
  //   return 0;

  // struct MuttWindow *win_sidebar = nc->global_data;

  // notify_observer_remove(NeoMutt->notify, sb_neomutt_observer, win_sidebar);

  // if (!nc->event_data || !nc->global_data)
  //   return -1;
  // if (nc->event_type != NT_WINDOW)
  //   return 0;
  // if (nc->event_subtype != NT_INDEX_MAILBOX)
  //   return 0;

  // struct MuttWindow *win_sidebar = nc->global_data;
  // struct SidebarWindowData *sb_data = win_sidebar->wdata;
  // if (!sb_data)
  //   return -1;

  // struct EventIndexDlg *eid = nc->event_data;
  // if (!eid)
  //   return -1;

  // win_sidebar: flag WA_RECALC
  // sb_data: update ptr to current Mailbox
  // eid: don't need this, wait for call to calc()

  return 0;
}

/**
 * sb_account_observer - XXX
 */
int sb_account_observer(struct MuttWindow *win, struct EventAccount *ec)
{
  // NT_ACCOUNT
  //   NT_ACCOUNT_ADD
  //   NT_ACCOUNT_REMOVE

  // if (nc->event_type == NT_ACCOUNT)
  // {
  //   struct EventAccount *ea = nc->event_data;
  //   return sb_account(win, ea);
  // }

  return 0;
}

/**
 * sb_color_observer - XXX
 */
int sb_color_observer(struct MuttWindow *win, struct EventColor *ec)
{
  // NT_COLOR
  //   sidebar_*
  //   indicator
  //   normal
  return 0;
}

/**
 * sb_command_observer - XXX
 */
int sb_command_observer(struct MuttWindow *win, struct Command *cmd)
{
  // NT_COMMAND
  //   sidebar_whitelist
  //   unsidebar_whitelist
  return 0;
}

/**
 * sb_config_observer - XXX
 */
int sb_config_observer(struct MuttWindow *win, struct EventConfig *ec)
{
  // NT_CONFIG
  //   sidebar_*
  //   ascii_chars
  //   folder
  //   spoolfile
  //   status_on_top

  // if (!nc->event_data || !nc->global_data)
  //   return -1;
  // if (nc->event_type != NT_CONFIG)
  //   return 0;

  // struct MuttWindow *win = nc->global_data;
  // struct EventConfig *ec = nc->event_data;

  // if (mutt_str_strncmp(ec->name, "sidebar_", 8) != 0)
  //   return 0;

  // bool repaint = false;

  // if (win->state.visible != C_SidebarVisible)
  // {
  //   window_set_visible(win, C_SidebarVisible);
  //   repaint = true;
  // }

  // if (win->req_cols != C_SidebarWidth)
  // {
  //   win->req_cols = C_SidebarWidth;
  //   repaint = true;
  // }

  // struct MuttWindow *parent = win->parent;
  // struct MuttWindow *first = TAILQ_FIRST(&parent->children);

  // if ((C_SidebarOnRight && (first == win)) || (!C_SidebarOnRight && (first != win)))
  // {
  //   // Swap the Sidebar and the Container of the Index/Pager
  //   TAILQ_REMOVE(&parent->children, first, entries);
  //   TAILQ_INSERT_TAIL(&parent->children, first, entries);
  //   repaint = true;
  // }

  // if (repaint)
  // {
  //   mutt_debug(LL_NOTIFY, "repaint sidebar\n");
  //   mutt_window_reflow(MuttDialogWindow);
  //   mutt_menu_set_current_redraw_full();
  // }

  return 0;
}

/**
 * sb_mailbox_observer - XXX
 */
int sb_mailbox_observer(struct MuttWindow *win, struct EventMailbox *ec)
{
  // NT_MAILBOX
  //   NT_MAILBOX_ADD
  //   NT_MAILBOX_REMOVE
  //   NT_MAILBOX_CLOSED
  //   NT_MAILBOX_INVALID
  //   NT_MAILBOX_RESORT
  //   NT_MAILBOX_UPDATE
  //   NT_MAILBOX_UNTAG

  // struct EventMailbox *em = nc->event_data;
  // struct Mailbox *m = em->mailbox;
  // struct SidebarWinData *data = win->wdata;
  // struct SidebarAccountView *av = sb_account_find(data, m->account);

  // if (nc->event_subtype == NT_MAILBOX_ADD)
  //   sb_mailbox_add(av, m);
  // else if (nc->event_subtype == NT_MAILBOX_REMOVE)
  //   sb_mailbox_remove(av, m);

  // if (nc->event_type == NT_MAILBOX)
  // {
  //   return sb_mailbox(win, nc);
  // }

  return 0;
}

/**
 * sb_neomutt_observer - XXX
 */
int sb_neomutt_observer(struct NotifyCallback *nc)
{
  if (!nc)
    return -1;

  struct MuttWindow *win = nc->global_data;

  switch (nc->event_type)
  {
    case NT_ACCOUNT:
      return sb_account_observer(win, nc->event_data);
    case NT_COLOR:
      return sb_color_observer(win, nc->event_data);
    case NT_COMMAND:
      return sb_command_observer(win, nc->event_data);
    case NT_CONFIG:
      return sb_config_observer(win, nc->event_data);
    case NT_MAILBOX:
      return sb_mailbox_observer(win, nc->event_data);
    default:
      return 0;
  }
}
