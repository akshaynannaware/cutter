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

#ifndef __CUT_SUB_PROCESS_GROUP_H__
#define __CUT_SUB_PROCESS_GROUP_H__

#include <glib-object.h>
#include <cutter/cut-public.h>

G_BEGIN_DECLS

#define CUT_TYPE_SUB_PROCESS_GROUP            (cut_sub_process_group_get_type ())
#define CUT_SUB_PROCESS_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CUT_TYPE_SUB_PROCESS_GROUP, CutSubProcessGroup))
#define CUT_SUB_PROCESS_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CUT_TYPE_SUB_PROCESS_GROUP, CutSubProcessGroupClass))
#define CUT_IS_SUB_PROCESS_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUT_TYPE_SUB_PROCESS_GROUP))
#define CUT_IS_SUB_PROCESS_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CUT_TYPE_SUB_PROCESS_GROUP))
#define CUT_SUB_PROCESS_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CUT_TYPE_SUB_PROCESS_GROUP, CutSubProcessGroupClass))

typedef struct _CutSubProcessGroupClass    CutSubProcessGroupClass;

struct _CutSubProcessGroup
{
    GObject object;
};

struct _CutSubProcessGroupClass
{
    GObjectClass parent_class;
};

GType           cut_sub_process_group_get_type      (void) G_GNUC_CONST;

CutSubProcessGroup *cut_sub_process_group_new          (CutTestContext *test_context);

CutTestContext *cut_sub_process_group_get_test_context (CutSubProcessGroup *sub_process_group);
void            cut_sub_process_group_set_test_context (CutSubProcessGroup *sub_process_group,
                                                        CutTestContext *test_context);


G_END_DECLS

#endif /* __CUT_SUB_PROCESS_GROUP_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
