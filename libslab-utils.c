#include "libslab-utils.h"

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <string.h>
#include <gconf/gconf-value.h>
#include <libgnome/gnome-url.h>

#define DESKTOP_ITEM_TERMINAL_EMULATOR_FLAG "TerminalEmulator"
#define ALTERNATE_DOCPATH_KEY               "DocPath"

gboolean
libslab_gtk_image_set_by_id (GtkImage *image, const gchar *id)
{
	GdkPixbuf *pixbuf;

	gint size;
	gint width;
	gint height;

	GtkIconTheme *icon_theme;

	gboolean found;

	gchar *tmp;


	if (! id)
		return FALSE;

	g_object_get (G_OBJECT (image), "icon-size", & size, NULL);

	if (size == GTK_ICON_SIZE_INVALID)
		size = GTK_ICON_SIZE_DND;

	gtk_icon_size_lookup (size, & width, & height);

	if (g_path_is_absolute (id)) {
		pixbuf = gdk_pixbuf_new_from_file_at_size (id, width, height, NULL);

		found = (pixbuf != NULL);

		if (found) {
			gtk_image_set_from_pixbuf (image, pixbuf);

			g_object_unref (pixbuf);
		}
		else
			gtk_image_set_from_stock (image, "gtk-missing-image", size);
	}
	else {
		tmp = g_strdup (id);

		if ( /* file extensions are not copesetic with loading by "name" */
			g_str_has_suffix (tmp, ".png") ||
			g_str_has_suffix (tmp, ".svg") ||
			g_str_has_suffix (tmp, ".xpm")
		)

			tmp [strlen (tmp) - 4] = '\0';

		if (gtk_widget_has_screen (GTK_WIDGET (image)))
			icon_theme = gtk_icon_theme_get_for_screen (
				gtk_widget_get_screen (GTK_WIDGET (image)));
		else
			icon_theme = gtk_icon_theme_get_default ();

		found = gtk_icon_theme_has_icon (icon_theme, tmp);

		if (found)
			gtk_image_set_from_icon_name (image, tmp, size);
		else
			gtk_image_set_from_stock (image, "gtk-missing-image", size);

		g_free (tmp);
	}

	return found;
}

GnomeDesktopItem *
libslab_gnome_desktop_item_new_from_unknown_id (const gchar *id)
{
	GnomeDesktopItem *item;

	GError *error = NULL;


	if (! id)
		return NULL;

	item = gnome_desktop_item_new_from_uri (id, 0, & error);

	if (! error)
		return item;
	else {
		g_error_free (error);
		error = NULL;
	}

	item = gnome_desktop_item_new_from_file (id, 0, & error);

	if (! error)
		return item;
	else {
		g_error_free (error);
		error = NULL;
	}

	item = gnome_desktop_item_new_from_basename (id, 0, & error);

	if (! error)
		return item;
	else {
		g_error_free (error);
		error = NULL;
	}

	return NULL;
}

gboolean
libslab_gnome_desktop_item_launch_default (GnomeDesktopItem *item)
{
	GError *error = NULL;

	if (! item)
		return FALSE;

	gnome_desktop_item_launch (item, NULL, GNOME_DESKTOP_ITEM_LAUNCH_ONLY_ONE, & error);

	if (error) {
		g_warning ("error launching %s [%s]\n",
			gnome_desktop_item_get_location (item), error->message);

		g_error_free (error);

		return FALSE;
	}

	return TRUE;
}

gchar *
libslab_gnome_desktop_item_get_docpath (GnomeDesktopItem *item)
{
	gchar *path;

	path = g_strdup (gnome_desktop_item_get_localestring (item, GNOME_DESKTOP_ITEM_DOC_PATH));

	if (! path)
		path = g_strdup (gnome_desktop_item_get_localestring (item, ALTERNATE_DOCPATH_KEY));

	return path;
}

gboolean
libslab_gnome_desktop_item_open_help (GnomeDesktopItem *item)
{
	gchar *doc_path;
	gchar *help_uri;

	GError *error = NULL;

	gboolean retval = FALSE;


	if (! item)
		return retval;

	doc_path = libslab_gnome_desktop_item_get_docpath (item);

	if (doc_path) {
		help_uri = g_strdup_printf ("ghelp:%s", doc_path);

		gnome_url_show (help_uri, & error);

		if (error) {
			g_warning ("error opening %s [%s]\n", help_uri, error->message);

			g_error_free (error);

			retval = FALSE;
		}
		else
			retval = TRUE;

		g_free (help_uri);
		g_free (doc_path);
	}

	return retval;
}

guint32
libslab_get_current_time_millis ()
{
	GTimeVal t_curr;

	g_get_current_time (& t_curr);

	return 1000L * t_curr.tv_sec + t_curr.tv_usec / 1000L;
}

gint
libslab_strcmp (const gchar *a, const gchar *b)
{
	if (! a && ! b)
		return 0;

	if (! a)
		return strcmp ("", b);

	if (! b)
		return strcmp (a, "");

	return strcmp (a, b);
}

gint
libslab_strlen (const gchar *a)
{
	if (! a)
		return 0;

	return strlen (a);
}

gpointer
libslab_get_gconf_value (const gchar *key)
{
	GConfClient *client;
	GConfValue  *value;
	GError      *error = NULL;

	gpointer retval = NULL;

	GList  *list;
	GSList *slist;

	GConfValue *value_i;
	GSList     *node;


	client = gconf_client_get_default ();
	value  = gconf_client_get (client, key, & error);

	if (error || ! value)
		libslab_handle_g_error (& error, "%s: error getting %s", G_STRFUNC, key);
	else {
		switch (value->type) {
			case GCONF_VALUE_STRING:
				retval = (gpointer) g_strdup (gconf_value_get_string (value));
				break;

			case GCONF_VALUE_INT:
				retval = GINT_TO_POINTER (gconf_value_get_int (value));
				break;

			case GCONF_VALUE_BOOL:
				retval = GINT_TO_POINTER (gconf_value_get_bool (value));
				break;

			case GCONF_VALUE_LIST:
				list = NULL;
				slist = gconf_value_get_list (value);

				for (node = slist; node; node = node->next) {
					value_i = (GConfValue *) node->data;

					if (value_i->type == GCONF_VALUE_STRING)
						list = g_list_append (
							list, g_strdup (
								gconf_value_get_string (value_i)));
					else if (value_i->type == GCONF_VALUE_INT)
						list = g_list_append (
							list, GINT_TO_POINTER (
								gconf_value_get_int (value_i)));
					else
						;
				}

				retval = (gpointer) list;

				break;

			default:
				break;
		}
	}

	g_object_unref (client);

	if (value)
		gconf_value_free (value);

	return retval;
}

void
libslab_set_gconf_value (const gchar *key, gconstpointer data)
{
	GConfClient *client;
	GConfValue  *value;

	GConfValueType type;
	GConfValueType list_type;

	GSList *slist = NULL;

	GError *error = NULL;

	GConfValue *value_i;
	GList      *node;


	client = gconf_client_get_default ();
	value  = gconf_client_get (client, key, & error);

	if (error) {
		libslab_handle_g_error (&error, "%s: error getting %s", G_STRFUNC, key);

		goto exit;
	}

	type = value->type;
	list_type = ((type == GCONF_VALUE_LIST) ?
		gconf_value_get_list_type (value) : GCONF_VALUE_INVALID);

	gconf_value_free (value);
	value = gconf_value_new (type);

	if (type == GCONF_VALUE_LIST)
		gconf_value_set_list_type (value, list_type);

	switch (type) {
		case GCONF_VALUE_STRING:
			gconf_value_set_string (value, g_strdup ((gchar *) data));
			break;

		case GCONF_VALUE_INT:
			gconf_value_set_int (value, GPOINTER_TO_INT (data));
			break;

		case GCONF_VALUE_BOOL:
			gconf_value_set_bool (value, GPOINTER_TO_INT (data));
			break;

		case GCONF_VALUE_LIST:
			for (node = (GList *) data; node; node = node->next) {
				value_i = gconf_value_new (list_type);

				if (list_type == GCONF_VALUE_STRING)
					gconf_value_set_string (value_i, (const gchar *) node->data);
				else if (list_type == GCONF_VALUE_INT)
					gconf_value_set_int (value_i, GPOINTER_TO_INT (node->data));
				else
					g_assert_not_reached ();

				slist = g_slist_append (slist, value_i);
			}

			gconf_value_set_list_nocopy (value, slist);

			break;

		default:
			break;
	}

	gconf_client_set (client, key, value, & error);

	if (error)
		libslab_handle_g_error (&error, "%s: error setting %s", G_STRFUNC, key);

exit:

	gconf_value_free (value);
	g_object_unref (client);
}

guint
libslab_gconf_notify_add (const gchar *key, GConfClientNotifyFunc callback, gpointer user_data)
{
	GConfClient *client;
	guint        conn_id;

	GError *error = NULL;


	client  = gconf_client_get_default ();
	conn_id = gconf_client_notify_add (client, key, callback, user_data, NULL, & error);

	if (error)
		libslab_handle_g_error (
			& error, "%s: error adding gconf notify for (%s)", G_STRFUNC, key);

	g_object_unref (client);

	return conn_id;
}

void
libslab_gconf_notify_remove (guint conn_id)
{
	GConfClient *client;

	GError *error = NULL;


	if (conn_id == 0)
		return;

	client = gconf_client_get_default ();
	gconf_client_notify_remove (client, conn_id);

	if (error)
		libslab_handle_g_error (
			& error, "%s: error removing gconf notify", G_STRFUNC);

	g_object_unref (client);
}

void
libslab_handle_g_error (GError **error, const gchar *msg_format, ...)
{
	gchar   *msg;
	va_list  args;


	va_start (args, msg_format);
	msg = g_strdup_vprintf (msg_format, args);
	va_end (args);

	if (*error) {
		g_log (
			G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
			"\nGError raised: [%s]\nuser_message: [%s]\n", (*error)->message, msg);

		g_error_free (*error);

		*error = NULL;
	}
	else
		g_log (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "\nerror raised: [%s]\n", msg);

	g_free (msg);
}

gboolean
libslab_desktop_item_is_a_terminal (const gchar *uri)
{
	GnomeDesktopItem *d_item;
	const gchar      *categories;

	gboolean is_terminal = FALSE;


	d_item = libslab_gnome_desktop_item_new_from_unknown_id (uri);

	if (! d_item)
		return FALSE;

	categories = gnome_desktop_item_get_string (d_item, GNOME_DESKTOP_ITEM_CATEGORIES);

	is_terminal = (categories && strstr (categories, DESKTOP_ITEM_TERMINAL_EMULATOR_FLAG));

	gnome_desktop_item_unref (d_item);

	return is_terminal;
}

gboolean
libslab_desktop_item_is_logout (const gchar *uri)
{
	GnomeDesktopItem *d_item;
	gboolean is_logout = FALSE;


	d_item = libslab_gnome_desktop_item_new_from_unknown_id (uri);

	if (! d_item)
		return FALSE;

	is_logout = strstr ("Logout", gnome_desktop_item_get_string (d_item, GNOME_DESKTOP_ITEM_NAME)) != NULL;

	gnome_desktop_item_unref (d_item);

	return is_logout;
}

gboolean
libslab_desktop_item_is_lockscreen (const gchar *uri)
{
	GnomeDesktopItem *d_item;
	gboolean is_logout = FALSE;


	d_item = libslab_gnome_desktop_item_new_from_unknown_id (uri);

	if (! d_item)
		return FALSE;

	is_logout = strstr ("Lock Screen", gnome_desktop_item_get_string (d_item, GNOME_DESKTOP_ITEM_NAME)) != NULL;

	gnome_desktop_item_unref (d_item);

	return is_logout;
}

gchar *
libslab_string_replace_once (const gchar *string, const gchar *key, const gchar *value)
{
	GString *str_built;
	gint pivot;


	pivot = strstr (string, key) - string;

	str_built = g_string_new_len (string, pivot);
	g_string_append (str_built, value);
	g_string_append (str_built, & string [pivot + strlen (key)]);

	return g_string_free (str_built, FALSE);
}

void
libslab_spawn_command (const gchar *cmd)
{
	gchar **argv;

	GError *error = NULL;


	if (! cmd || strlen (cmd) < 1)
		return;

	argv = g_strsplit (cmd, " ", -1);

	g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, & error);

	if (error)
		libslab_handle_g_error (& error, "%s: error spawning [%s]", G_STRFUNC, cmd);

	g_strfreev (argv);
}
