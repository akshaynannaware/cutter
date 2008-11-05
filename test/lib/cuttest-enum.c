/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2008  Kouhei Sutou <kou@cozmixng.org>
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "cuttest-enum.h"

GType
cuttest_flags_get_type (void)
{
    static GType flags_type = 0;

    if (flags_type == 0) {
        static const GFlagsValue values[] = {
            {CUTTEST_FLAG_FIRST, "CUTTEST_FLAG_FIRST", "first"},
            {CUTTEST_FLAG_SECOND, "CUTTEST_FLAG_SECOND", "second"},
            {CUTTEST_FLAG_THIRD, "CUTTEST_FLAG_THIRD", "third"},
            {0, NULL, NULL}
        };
        flags_type = g_flags_register_static("CuttestFlags", values);
    }

    return flags_type;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
