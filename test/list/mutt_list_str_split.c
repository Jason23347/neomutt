/**
 * @file
 * Test code for mutt_list_str_split()
 *
 * @authors
 * Copyright (C) 2019 Richard Russon <rich@flatcap.org>
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

#define TEST_NO_MAIN
#include "config.h"
#include "acutest.h"
#include "mutt/lib.h"

void print_compared_list(struct ListHead expected, struct ListHead actual)
{
  struct ListNode *np = NULL;

  TEST_MSG("Expected:");
  STAILQ_FOREACH(np, &expected, entries)
  {
    TEST_MSG("* '%s'", np->data);
  }

  TEST_MSG("Actual:");
  STAILQ_FOREACH(np, &actual, entries)
  {
    TEST_MSG("* '%s'", np->data);
  }
}

void test_mutt_list_str_split(void)
{
  // size_t mutt_list_str_split(const char *src, char sep, struct ListHead *head);

  {
    struct ListHead head = STAILQ_HEAD_INITIALIZER(head);
    size_t count = mutt_list_str_split(NULL, ',', &head);
    TEST_CHECK(count == 0);
    TEST_CHECK(STAILQ_EMPTY(&head));
  }

  char *one_word = "hello";
  char *two_words = "hello world";
  char *words = "hello neomutt world! what's up?";
  char *ending_sep = "hello world ";
  char *starting_sep = " hello world";
  char *other_sep = "hello,world";
  char *empty = "";

  { // Check NULL conditions
    size_t count = 0;

    struct ListHead retval1 = STAILQ_HEAD_INITIALIZER(retval1);
    count = mutt_list_str_split(NULL, ' ', &retval1);
    if (!TEST_CHECK(STAILQ_EMPTY(&retval1)) || (count != 0))
      TEST_MSG("Expected: empty");

    struct ListHead retval2 = STAILQ_HEAD_INITIALIZER(retval2);
    count = mutt_list_str_split(empty, ' ', &retval2);
    if (!TEST_CHECK(STAILQ_EMPTY(&retval2)) || (count != 0))
      TEST_MSG("Expected: empty");
  }

  { // Check different words
    struct ListHead retval1 = STAILQ_HEAD_INITIALIZER(retval1);
    struct ListHead retval2 = STAILQ_HEAD_INITIALIZER(retval2);
    struct ListHead retval3 = STAILQ_HEAD_INITIALIZER(retval3);
    struct ListHead retval4 = STAILQ_HEAD_INITIALIZER(retval4);
    struct ListHead retval5 = STAILQ_HEAD_INITIALIZER(retval5);
    struct ListHead retval6 = STAILQ_HEAD_INITIALIZER(retval6);

    size_t count1 = mutt_list_str_split(one_word, ' ', &retval1);
    size_t count2 = mutt_list_str_split(two_words, ' ', &retval2);
    size_t count3 = mutt_list_str_split(words, ' ', &retval3);
    size_t count4 = mutt_list_str_split(ending_sep, ' ', &retval4);
    size_t count5 = mutt_list_str_split(starting_sep, ' ', &retval5);
    size_t count6 = mutt_list_str_split(other_sep, ',', &retval6);

    struct ListHead expectedval1 = STAILQ_HEAD_INITIALIZER(expectedval1);
    mutt_list_insert_tail(&expectedval1, "hello");
    if (!TEST_CHECK(mutt_list_compare(&expectedval1, &retval1)) || (count1 != 1))
      print_compared_list(expectedval1, retval1);

    struct ListHead expectedval2 = STAILQ_HEAD_INITIALIZER(expectedval2);
    mutt_list_insert_tail(&expectedval2, "hello");
    mutt_list_insert_tail(&expectedval2, "world");
    if (!TEST_CHECK(mutt_list_compare(&expectedval2, &retval2)) || (count2 != 2))
      print_compared_list(expectedval2, retval2);

    struct ListHead expectedval3 = STAILQ_HEAD_INITIALIZER(expectedval3);
    mutt_list_insert_tail(&expectedval3, "hello");
    mutt_list_insert_tail(&expectedval3, "neomutt");
    mutt_list_insert_tail(&expectedval3, "world!");
    mutt_list_insert_tail(&expectedval3, "what's");
    mutt_list_insert_tail(&expectedval3, "up?");
    if (!TEST_CHECK(mutt_list_compare(&expectedval3, &retval3)) || (count3 != 5))
      print_compared_list(expectedval3, retval3);

    struct ListHead expectedval4 = STAILQ_HEAD_INITIALIZER(expectedval4);
    mutt_list_insert_tail(&expectedval4, "hello");
    mutt_list_insert_tail(&expectedval4, "world");
    mutt_list_insert_tail(&expectedval4, "");
    if (!TEST_CHECK(mutt_list_compare(&expectedval4, &retval4)) || (count4 != 3))
      print_compared_list(expectedval4, retval4);

    struct ListHead expectedval5 = STAILQ_HEAD_INITIALIZER(expectedval5);
    mutt_list_insert_tail(&expectedval5, "");
    mutt_list_insert_tail(&expectedval5, "hello");
    mutt_list_insert_tail(&expectedval5, "world");
    if (!TEST_CHECK(mutt_list_compare(&expectedval5, &retval5)) || (count5 != 3))
      print_compared_list(expectedval5, retval5);

    struct ListHead expectedval6 = STAILQ_HEAD_INITIALIZER(expectedval6);
    mutt_list_insert_tail(&expectedval6, "hello");
    mutt_list_insert_tail(&expectedval6, "world");
    if (!TEST_CHECK(mutt_list_compare(&expectedval6, &retval6)) || (count6 != 2))
      print_compared_list(expectedval6, retval6);

    mutt_list_free(&retval1);
    mutt_list_free(&retval2);
    mutt_list_free(&retval3);
    mutt_list_free(&retval4);
    mutt_list_free(&retval5);
    mutt_list_free(&retval6);

    mutt_list_clear(&expectedval1);
    mutt_list_clear(&expectedval2);
    mutt_list_clear(&expectedval3);
    mutt_list_clear(&expectedval4);
    mutt_list_clear(&expectedval5);
    mutt_list_clear(&expectedval6);
  }
}
