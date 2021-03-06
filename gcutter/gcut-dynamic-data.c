/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2008-2019  Sutou Kouhei <kou@clear-code.com>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <gcutter.h>

#define GCUT_DYNAMIC_DATA_GET_PRIVATE(obj)                              \
    ((GCutDynamicDataPrivate *)                                         \
     gcut_dynamic_data_get_instance_private(GCUT_DYNAMIC_DATA(obj)))

typedef struct _Field
{
    GCutDynamicData *data;
    gchar *name;
    GType type;
    union {
        gpointer pointer;
        GType type;
        gchar character;
        gint integer;
        guint unsigned_integer;
        gint64 integer64;
        guint64 unsigned_integer64;
        gsize size;
        gboolean boolean;
        gdouble double_value;
    } value;
    GDestroyNotify free_function;
} Field;

typedef struct _GCutDynamicDataPrivate	GCutDynamicDataPrivate;
struct _GCutDynamicDataPrivate
{
    GHashTable *fields;
};

static Field *
field_new (GCutDynamicData *data, const gchar *name, GType type)
{
    Field *field;

    field = g_slice_new0(Field);
    field->data = data;
    field->name = g_strdup(name);
    field->type = type;

    return field;
}

static void
field_free (Field *field)
{
    if (field->value.pointer) {
        if (field->free_function) {
            field->free_function(field->value.pointer);
        } else if (G_TYPE_IS_BOXED(field->type)) {
            g_boxed_free(field->type, field->value.pointer);
        }
    }

    g_free(field->name);
    g_slice_free(Field, field);
}

static void
field_inspect (GString *string, gconstpointer data, gpointer user_data)
{
    const Field *field = data;

    switch (field->type) {
    case G_TYPE_CHAR:
        gcut_inspect_char(string, &(field->value.character), user_data);
        break;
    case G_TYPE_STRING:
        gcut_inspect_string(string, field->value.pointer, user_data);
        break;
    case G_TYPE_INT:
        gcut_inspect_int(string, &(field->value.integer), user_data);
        break;
    case G_TYPE_UINT:
        gcut_inspect_uint(string, &(field->value.unsigned_integer), user_data);
        break;
    case G_TYPE_INT64:
        gcut_inspect_int64(string, &(field->value.integer64), user_data);
        break;
    case G_TYPE_UINT64:
        gcut_inspect_uint64(string,
                            &(field->value.unsigned_integer64),
                            user_data);
        break;
    case G_TYPE_POINTER:
        gcut_inspect_pointer(string, field->value.pointer, user_data);
        break;
    case G_TYPE_BOOLEAN:
        gcut_inspect_boolean(string, &(field->value.boolean), user_data);
        break;
    case G_TYPE_DOUBLE:
        gcut_inspect_double(string, &(field->value.double_value), user_data);
        break;
    default:
        if (field->type == GCUT_TYPE_SIZE) {
            gcut_inspect_size(string, &(field->value.size), user_data);
        } else if (field->type == G_TYPE_GTYPE) {
            gcut_inspect_type(string, &(field->value.type), user_data);
        } else if (G_TYPE_IS_FLAGS(field->type)) {
            GType flags_type = field->type;
            gcut_inspect_flags(string,
                               &(field->value.unsigned_integer),
                               &flags_type);
        } else if (G_TYPE_IS_ENUM(field->type)) {
            GType enum_type = field->type;
            gcut_inspect_enum(string,
                              &(field->value.integer),
                              &enum_type);
        } else {
            g_string_append_printf(string,
                                   "[unsupported type: <%s>]",
                                   g_type_name(field->type));
        }
        break;
    }
}

static gboolean
field_equal (gconstpointer data1, gconstpointer data2)
{
    const Field *field1 = data1;
    const Field *field2 = data2;
    gboolean result = FALSE;
    gchar *error_field_name;
    gdouble error;

    if (field1 == field2)
        return TRUE;

    if (field1 == NULL || field2 == NULL)
        return FALSE;

    if (field1->type != field2->type)
        return FALSE;

    switch (field1->type) {
    case G_TYPE_CHAR:
        result = (field1->value.character == field2->value.character);
        break;
    case G_TYPE_STRING:
        result = g_str_equal(field1->value.pointer, field2->value.pointer);
        break;
    case G_TYPE_INT:
        result = (field1->value.integer == field2->value.integer);
        break;
    case G_TYPE_UINT:
        result = (field1->value.unsigned_integer ==
                  field2->value.unsigned_integer);
        break;
    case G_TYPE_INT64:
        result = (field1->value.integer64 == field2->value.integer64);
        break;
    case G_TYPE_UINT64:
        result = (field1->value.unsigned_integer64 ==
                  field2->value.unsigned_integer64);
        break;
    case G_TYPE_POINTER:
        result = (field1->value.pointer == field2->value.pointer);
        break;
    case G_TYPE_BOOLEAN:
        result = ((field1->value.boolean && field2->value.boolean) ||
                  (!field1->value.boolean && !field2->value.boolean));
        break;
    case G_TYPE_DOUBLE:
        error_field_name = g_strdup_printf("%s-error", field1->name);
        error = gcut_dynamic_data_get_double(field1->data, error_field_name,
                                             NULL);
        g_free(error_field_name);
        result = cut_utils_equal_double(field1->value.double_value,
                                        field2->value.double_value,
                                        error);
        break;
    default:
        if (field1->type == GCUT_TYPE_SIZE) {
            result = (field1->value.size == field2->value.size);
        } else if (field1->type == G_TYPE_GTYPE) {
            result = (field1->value.type == field2->value.type);
        } else if (G_TYPE_IS_FLAGS(field1->type)) {
            result = (field1->value.unsigned_integer ==
                      field2->value.unsigned_integer);
        } else if (G_TYPE_IS_ENUM(field1->type)) {
            result = (field1->value.integer ==
                      field2->value.integer);
        } else {
            g_warning("[unsupported type: <%s>]",
                      g_type_name(field1->type));
            result = FALSE;
        }
        break;
    }

    return result;
}

G_DEFINE_TYPE_WITH_PRIVATE(GCutDynamicData, gcut_dynamic_data, G_TYPE_OBJECT)

static void dispose        (GObject         *object);

static void
gcut_dynamic_data_class_init (GCutDynamicDataClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose      = dispose;
}

static void
gcut_dynamic_data_init (GCutDynamicData *data)
{
    GCutDynamicDataPrivate *priv = GCUT_DYNAMIC_DATA_GET_PRIVATE(data);

    priv->fields = g_hash_table_new_full(g_str_hash, g_str_equal,
                                         g_free,
                                         (GDestroyNotify)field_free);
}

static void
dispose (GObject *object)
{
    GCutDynamicDataPrivate *priv;

    priv = GCUT_DYNAMIC_DATA_GET_PRIVATE(object);
    if (priv->fields) {
        g_hash_table_unref(priv->fields);
        priv->fields = NULL;
    }

    G_OBJECT_CLASS(gcut_dynamic_data_parent_class)->dispose(object);
}

GQuark
gcut_dynamic_data_error_quark (void)
{
    return g_quark_from_static_string("gcut-dynamic-data-error-quark");
}

GCutDynamicData *
gcut_dynamic_data_new (const gchar *first_field_name, ...)
{
    GCutDynamicData *data;
    va_list args;

    va_start(args, first_field_name);
    data = gcut_dynamic_data_new_va_list(first_field_name, args);
    va_end(args);

    return data;
}

GCutDynamicData *
gcut_dynamic_data_new_va_list (const gchar *first_field_name, va_list args)
{
    GCutDynamicDataPrivate *priv;
    GCutDynamicData *data;
    const gchar *name;

    data = g_object_new(GCUT_TYPE_DYNAMIC_DATA, NULL);
    priv = GCUT_DYNAMIC_DATA_GET_PRIVATE(data);
    name = first_field_name;
    while (name) {
        Field *field;

        field = field_new(data, name, va_arg(args, GType));
        switch (field->type) {
        case G_TYPE_CHAR:
            field->value.character = va_arg(args, gint);
            break;
        case G_TYPE_STRING:
            field->value.pointer = g_strdup(va_arg(args, const gchar *));
            field->free_function = g_free;
            break;
        case G_TYPE_INT:
            field->value.integer = va_arg(args, gint);
            break;
        case G_TYPE_UINT:
            field->value.unsigned_integer = va_arg(args, guint);
            break;
        case G_TYPE_INT64:
            field->value.integer64 = va_arg(args, gint64);
            break;
        case G_TYPE_UINT64:
            field->value.unsigned_integer64 = va_arg(args, guint64);
            break;
        case G_TYPE_POINTER:
            field->value.pointer = va_arg(args, gpointer);
            field->free_function = va_arg(args, GDestroyNotify);
            break;
        case G_TYPE_BOOLEAN:
            field->value.boolean = va_arg(args, gboolean);
            break;
        case G_TYPE_DOUBLE:
            field->value.double_value = va_arg(args, gdouble);
            break;
        default:
            if (field->type == GCUT_TYPE_SIZE) {
                field->value.size = va_arg(args, gsize);
            } else if (field->type == G_TYPE_GTYPE) {
                field->value.type = va_arg(args, GType);
            } else if (G_TYPE_IS_FLAGS(field->type)) {
                field->value.unsigned_integer = va_arg(args, guint);
            } else if (G_TYPE_IS_ENUM(field->type)) {
                field->value.integer = va_arg(args, gint);
            } else if (G_TYPE_IS_BOXED(field->type)) {
                field->value.pointer = va_arg(args, gpointer);
            } else if (G_TYPE_IS_OBJECT(field->type)) {
                field->value.pointer = va_arg(args, gpointer);
                field->free_function = g_object_unref;
            } else {
                g_warning("unsupported type: <%s>",
                          g_type_name(field->type));
                field_free(field);
                field = NULL;
            }
            break;
        }
        if (!field)
            break;

        g_hash_table_insert(priv->fields, g_strdup(name), field);

        name = va_arg(args, const gchar *);
    }

    return data;
}

gchar *
gcut_dynamic_data_inspect (GCutDynamicData *data)
{
    GCutDynamicDataPrivate *priv;
    GString *string;
    gchar *inspected_fields;

    priv = GCUT_DYNAMIC_DATA_GET_PRIVATE(data);
    string = g_string_new(NULL);
    g_string_append_printf(string, "#<GCutDynamicData:0x%p ", data);
    inspected_fields = gcut_hash_table_inspect(priv->fields,
                                               gcut_inspect_string,
                                               field_inspect,
                                               NULL);
    g_string_append(string, inspected_fields);
    g_free(inspected_fields);
    g_string_append(string, ">");

    return g_string_free(string, FALSE);
}

gboolean
gcut_dynamic_data_equal (GCutDynamicData *data1, GCutDynamicData *data2)
{
    GCutDynamicDataPrivate *priv1;
    GCutDynamicDataPrivate *priv2;

    if (data1 == data2)
        return TRUE;

    if (data1 == NULL || data2 == NULL)
        return FALSE;

    priv1 = GCUT_DYNAMIC_DATA_GET_PRIVATE(data1);
    priv2 = GCUT_DYNAMIC_DATA_GET_PRIVATE(data2);
    return gcut_hash_table_equal(priv1->fields,
                                 priv2->fields,
                                 field_equal);
}

gboolean
gcut_dynamic_data_has_field (GCutDynamicData *data, const gchar *field_name)
{
    GCutDynamicDataPrivate *priv;
    Field *field;

    priv = GCUT_DYNAMIC_DATA_GET_PRIVATE(data);
    field = g_hash_table_lookup(priv->fields, field_name);
    return field ? TRUE : FALSE;
}

static Field *
lookup (GCutDynamicData *data, const gchar *field_name, GError **error)
{
    GCutDynamicDataPrivate *priv;
    Field *field;

    priv = GCUT_DYNAMIC_DATA_GET_PRIVATE(data);
    field = g_hash_table_lookup(priv->fields, field_name);
    if (!field) {
        g_set_error(error,
                    GCUT_DYNAMIC_DATA_ERROR,
                    GCUT_DYNAMIC_DATA_ERROR_NOT_EXIST,
                    "requested field doesn't exist: <%s>", field_name);
        return NULL;
    }

    return field;
}

gchar
gcut_dynamic_data_get_char (GCutDynamicData *data, const gchar *field_name,
                            GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return '\0';

    return field->value.character;
}

const gchar *
gcut_dynamic_data_get_string (GCutDynamicData *data, const gchar *field_name,
                              GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return NULL;

    return field->value.pointer;
}

GType
gcut_dynamic_data_get_data_type (GCutDynamicData *data, const gchar *field_name,
                                 GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return G_TYPE_INVALID;

    return field->value.type;
}

gint
gcut_dynamic_data_get_int (GCutDynamicData *data, const gchar *field_name,
                           GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0;

    return field->value.integer;
}

guint
gcut_dynamic_data_get_uint (GCutDynamicData *data, const gchar *field_name,
                            GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0;

    return field->value.unsigned_integer;
}

gint64
gcut_dynamic_data_get_int64 (GCutDynamicData *data, const gchar *field_name,
                             GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0;

    return field->value.integer64;
}

guint64
gcut_dynamic_data_get_uint64 (GCutDynamicData *data, const gchar *field_name,
                              GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0;

    return field->value.unsigned_integer64;
}

gsize
gcut_dynamic_data_get_size (GCutDynamicData *data, const gchar *field_name,
                            GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0;

    return field->value.size;
}

guint
gcut_dynamic_data_get_flags (GCutDynamicData *data, const gchar *field_name,
                             GError **error)
{
    return gcut_dynamic_data_get_uint(data, field_name, error);
}

gint
gcut_dynamic_data_get_enum (GCutDynamicData *data, const gchar *field_name,
                            GError **error)
{
    return gcut_dynamic_data_get_int(data, field_name, error);
}

gconstpointer
gcut_dynamic_data_get_pointer (GCutDynamicData *data, const gchar *field_name,
                               GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0;

    return field->value.pointer;
}

gconstpointer
gcut_dynamic_data_get_boxed (GCutDynamicData  *data,
                             const gchar      *field_name,
                             GError          **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0;

    return field->value.pointer;
}

gpointer
gcut_dynamic_data_get_object (GCutDynamicData  *data,
                              const gchar      *field_name,
                              GError          **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0;

    return field->value.pointer;
}

gboolean
gcut_dynamic_data_get_boolean (GCutDynamicData *data, const gchar *field_name,
                               GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0;

    return field->value.boolean;
}

gdouble
gcut_dynamic_data_get_double (GCutDynamicData *data, const gchar *field_name,
                              GError **error)
{
    Field *field;

    field = lookup(data, field_name, error);
    if (!field)
        return 0.0;

    return field->value.double_value;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
