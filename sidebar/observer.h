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

#ifndef MUTT_SIDEBAR_OBSERVER_H
#define MUTT_SIDEBAR_OBSERVER_H

struct EventAccount;
struct EventColor;
struct EventCommand;
struct EventConfig;
struct EventMailbox;
struct MuttWindow;
struct NotifyCallback;

int sb_account_observer(struct MuttWindow *win, struct EventAccount *ea);
int sb_color_observer  (struct MuttWindow *win, struct EventColor   *ec);
int sb_command_observer(struct MuttWindow *win, struct EventCommand *ec);
int sb_config_observer (struct MuttWindow *win, struct EventConfig  *ec);
int sb_mailbox_observer(struct MuttWindow *win, struct EventMailbox *em);

int sb_dialog_observer (struct NotifyCallback *nc);
int sb_neomutt_observer(struct NotifyCallback *nc);

#endif /* MUTT_SIDEBAR_OBSERVER_H */
