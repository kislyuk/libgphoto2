/* library.c
 *
 * Copyright (C) 2001 Lutz M�ller
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <gphoto2-library.h>
#include <gphoto2-core.h>
#include <gphoto2-frontend.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <time.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define _(String) (String)
#    define N_(String) (String)
#  endif
#else
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include "library.h"
#include "konica.h"

#define CHECK(r) {gint ret = (r); if (ret < 0) return (ret);}

static gchar* konica_results[] = {
/* FOCUSING_ERROR		*/ N_("Focusing error"),
/* IRIS_ERROR			*/ N_("Iris error"),
/* STROBE_ERROR			*/ N_("Strobe error"),
/* EEPROM_CHECKSUM_ERROR	*/ N_("EEPROM checksum error"),
/* INTERNAL_ERROR1		*/ N_("Internal error (1)"),
/* INTERNAL_ERROR2		*/ N_("Internal error (2)"),
/* NO_CARD_PRESENT		*/ N_("No card present"),
/* CARD_NOT_SUPPORTED		*/ N_("Card not supported"),
/* CARD_REMOVED_DURING_ACCESS	*/ N_("Card removed during access"),
/* IMAGE_NUMBER_NOT_VALID	*/ N_("Image number not valid"),
/* CARD_CAN_NOT_BE_WRITTEN	*/ N_("Card cannot be written"),
/* CARD_IS_WRITE_PROTECTED	*/ N_("Card is write protected"),
/* NO_SPACE_LEFT_ON_CARD	*/ N_("No space left on card"),
/* IMAGE_PROTECTED		*/ N_("No image erased as image protected"),
/* LIGHT_TOO_DARK		*/ N_("Light too dark"),
/* AUTOFOCUS_ERROR		*/ N_("Autofocus error"),
/* SYSTEM_ERROR			*/ N_("System error"),
/* ILLEGAL_PARAMETER		*/ N_("Illegal parameter"),
/* COMMAND_CANNOT_BE_CANCELLED	*/ N_("Command cannot be cancelled"),
/* LOCALIZATION_DATA_EXCESS	*/ N_("Localization data excess"),
/* LOCALIZATION_DATA_CORRUPT	*/ N_("Localization data corrupt"),
/* UNSUPPORTED_COMMAND		*/ N_("Unsupported command"),
/* OTHER_COMMAND_EXECUTING	*/ N_("Other command executing"),
/* COMMAND_ORDER_ERROR		*/ N_("Command order error"),
/* UNKNOWN_ERROR		*/ N_("Unknown error")
};

static struct {
	const gchar *model;
	int image_id_long;
	int vendor;
	int product;
	int inep;
	int outep;
} konica_cameras[] = {
	{"Konica Q-EZ",        0, 0, 0, 0, 0},
	{"Konica Q-M100",      0, 0, 0, 0, 0},
	{"Konica Q-M100V",     0, 0, 0, 0, 0},
	{"Konica Q-M200",      1, 0, 0, 0, 0},
	{"HP PhotoSmart C20",  0, 0, 0, 0, 0},
	{"HP PhotoSmart C30",  0, 0, 0, 0, 0},
	{"HP PhotoSmart C200", 1, 0, 0, 0, 0},
	{NULL,                 0, 0, 0, 0, 0}
};

typedef struct {
	int image_id_long;
} KonicaData;
			

static int localization_file_read (Camera* camera, const char* file_name,
				   unsigned char** data, long int* data_size);


static int
file_list_func (CameraFilesystem *fs, const char *folder, CameraList *list,
		void *data)
{
	KStatus status;
        guint           i;
        guchar*         information_buffer = NULL;
        guint           information_buffer_size;
        guint           exif_size;
        gboolean        protected;
        gulong          image_id;
        char filename[1024];
	int result;
	Camera *camera = data;
	KonicaData *kd = camera->camlib_data;

	/*
	 * We can't get the filename from the camera. 
	 * But we decide to call the images %6i.jpeg', with the image id as
	 * parameter. Therefore, let's get the image ids.
	 */
	CHECK (k_get_status (camera->port, &status));
        for (i = 0; i < status.pictures; i++) {

		/* Get information */
		result = k_get_image_information (camera->port,
			kd->image_id_long, i + 1, &image_id, &exif_size,
			&protected, &information_buffer,
			&information_buffer_size);
		g_free (information_buffer);
		information_buffer = NULL;
		CHECK (result);

		/* Append to the list */
		sprintf (filename, "%06i.jpeg", (int) image_id);
		CHECK (gp_list_append (list, filename, NULL));
        }

        return (GP_OK);
}

int 
camera_id (CameraText* id)
{
	strcpy (id->text, "konica");

	return (GP_OK);
}

int 
camera_abilities (CameraAbilitiesList* list)
{
	int i;
	CameraAbilities *a;

	for (i = 0; konica_cameras [i].model; i++) {
		gp_abilities_new (&a);
		strcpy (a->model, konica_cameras [i].model);
		if (konica_cameras [i].vendor) {
			a->port = GP_PORT_USB;
			a->usb_vendor  = konica_cameras [i].vendor;
			a->usb_product = konica_cameras [i].product;
		} else {
			a->port = GP_PORT_SERIAL;
			a->speed[0]	= 300;
			a->speed[1] 	= 600;
			a->speed[2]	= 1200;
			a->speed[3]	= 2400;
			a->speed[4]	= 4800;
			a->speed[5]	= 9600;
			a->speed[6]	= 19200;
			a->speed[7]	= 38400;
			a->speed[8]	= 57600;
			a->speed[9]	= 115200;
			a->speed[10]	= 0;
		}
		a->operations = GP_OPERATION_CONFIG |
			        GP_OPERATION_CAPTURE_IMAGE |
				GP_OPERATION_CAPTURE_PREVIEW;
		a->file_operations = GP_FILE_OPERATION_DELETE |
			             GP_FILE_OPERATION_PREVIEW;
		a->folder_operations = GP_FOLDER_OPERATION_CONFIG |
			               GP_FOLDER_OPERATION_DELETE_ALL;
		gp_abilities_list_append (list, a);
        }

	return (GP_OK);
}

static gint
init_serial_connection (Camera *camera)
{
	gint i;
	gboolean bit_flag_8_bits, bit_flag_stop_2_bits, bit_flag_parity_on;
	gboolean bit_flag_parity_odd, bit_flag_use_hw_flow_control;
	guint test_bit_rate [10] = {9600, 115200, 57600, 38400, 19200, 4800,
		                    2400, 1200, 600, 300};
	guint bit_rate [10] = {300, 600, 1200, 2400, 4800, 9600, 19200, 38400,
		               57600, 115200};
	gboolean bit_rate_supported [10] = {FALSE, FALSE, FALSE, FALSE, FALSE,
		                            FALSE, FALSE, FALSE, FALSE,  FALSE};
	gp_port_settings settings;
	guint speed;

	gp_debug_printf (GP_DEBUG_LOW, "konica", "*** ENTER: "
			 "init_serial_connection ***");

	CHECK (gp_port_settings_get (camera->port, &settings));

	/* Speed: 
	 * 
	 * We are given 0: 
	 * We'll try all possibilities, starting with 9600, then trying all 
	 * other values. Once we found the camera's current speed, we'll set 
	 * it to the highest value supported by the camera.
	 *
	 * We are not given 0:
	 * We will first try to communicate with the camera with the given
	 * speed. If we fail, we will try all possibilities. If the given
	 * speed does not equal the current speed, we will change the 
	 * current speed to the given one.
	 */

	if (camera->port_info->speed) {
		settings.serial.speed = camera->port_info->speed;
		CHECK (gp_port_settings_set (camera->port, settings));
		CHECK (gp_port_open (camera->port));
		if (k_init (camera->port) == GP_OK)
			return (GP_OK);
	}

	for (i = 0; i < 10; i++) {
		gp_debug_printf (GP_DEBUG_LOW, "konica", "-> Testing speed "
				 "%i.\n", test_bit_rate [i]);
		settings.serial.speed = test_bit_rate [i];
		gp_port_settings_set (camera->port, settings);
		CHECK (gp_port_open (camera->port));
		if (k_init (camera->port) == GP_OK)
			break;
		CHECK (gp_port_close (camera->port));
	}
	if (!camera->port_info->speed && (i == 1)) {
		CHECK (gp_port_close (camera->port));
		return (GP_OK);
	}
	if (i == 10)
		return (GP_ERROR_IO);

	/* What are the camera's abilities? */
	gp_debug_printf (GP_DEBUG_LOW, "konica", "-> Getting IO "
			 "capabilities...\n");
	CHECK (k_get_io_capability (camera->port, &bit_rate_supported [0], 
			&bit_rate_supported [1], &bit_rate_supported [2],
			&bit_rate_supported [3], &bit_rate_supported [4],
			&bit_rate_supported [5], &bit_rate_supported [6],
			&bit_rate_supported [7], &bit_rate_supported [8],
			&bit_rate_supported [9], &bit_flag_8_bits,
			&bit_flag_stop_2_bits, &bit_flag_parity_on,
			&bit_flag_parity_odd, &bit_flag_use_hw_flow_control));

	if (!camera->port_info->speed) {

		/* Set the highest possible speed */
		for (i = 9; i >= 0; i--)
			if (bit_rate_supported [i])
				break;
		if (i < 0) {
			CHECK (gp_port_close (camera->port));
			return (GP_ERROR_IO_SERIAL_SPEED);
		}
		camera->port_info->speed = bit_rate [i];
	} else {

		/* Does the camera allow us to set the bit rate to the 
		 * given one?
		 */
		speed = camera->port_info->speed;
		if (    ((speed ==    300) && (!bit_rate_supported[0])) ||
			((speed ==    600) && (!bit_rate_supported[1])) ||
			((speed ==   1200) && (!bit_rate_supported[2])) ||
			((speed ==   2400) && (!bit_rate_supported[3])) ||
			((speed ==   4800) && (!bit_rate_supported[4])) ||
			((speed ==   9600) && (!bit_rate_supported[5])) ||
			((speed ==  19200) && (!bit_rate_supported[6])) ||
			((speed ==  38400) && (!bit_rate_supported[7])) ||
			((speed ==  57600) && (!bit_rate_supported[8])) ||
			((speed == 115200) && (!bit_rate_supported[9])) ||
			((speed !=    300) && (speed !=    600) &&
			 (speed !=   1200) && (speed !=   2400) &&
			 (speed !=   4800) && (speed !=   9600) &&
			 (speed !=  19200) && (speed !=  38400) &&
			 (speed !=  57600) && (speed != 115200))) {
			CHECK (gp_port_close (camera->port));
			return (GP_ERROR_IO_SERIAL_SPEED);
		}
	}

	/* Request the new speed */
	gp_debug_printf (GP_DEBUG_LOW, "konica", "-> Setting speed to "
			 "%i...\n", camera->port_info->speed);
	CHECK (k_set_io_capability (camera->port, camera->port_info->speed,
				    TRUE, FALSE, FALSE, FALSE, FALSE)); 
	
	/* Reconnect */
	settings.serial.speed = camera->port_info->speed;
	CHECK (gp_port_settings_set (camera->port, settings));
	CHECK (k_init (camera->port));
	CHECK (gp_port_close (camera->port));

	gp_debug_printf (GP_DEBUG_LOW, "konica", "*** EXIT: "
			 "init_serial_connection ***");
	
	return (GP_OK);
}

static int
set_info_func (CameraFilesystem *fs, const char *folder, const char *file,
	       CameraFileInfo *info, void *data)
{
	Camera *camera = data;
	KonicaData *kd = camera->camlib_data;
	char tmp[7];
	gboolean protected;
	gulong image_id;

	gp_debug_printf (GP_DEBUG_LOW, "konica", "*** Entering set_info_func "
						 "***");

	if (info->file.fields & (GP_FILE_INFO_SIZE | GP_FILE_INFO_TYPE |
				 GP_FILE_INFO_NAME))
		return (GP_ERROR_NOT_SUPPORTED);
	if (info->preview.fields & (GP_FILE_INFO_SIZE | GP_FILE_INFO_TYPE |
				    GP_FILE_INFO_NAME |
				    GP_FILE_INFO_PERMISSIONS))
		return (GP_ERROR_NOT_SUPPORTED);

	/* Permissions? */
	if (info->file.fields & GP_FILE_INFO_PERMISSIONS) {
		strncpy (tmp, file, 6);
		tmp[6] = '\0';
		image_id = atol (tmp);
		if (info->file.permissions & GP_FILE_PERM_DELETE)
			protected = FALSE;
		else
			protected = TRUE;
		CHECK (k_set_protect_status (camera->port, kd->image_id_long,
					     image_id, protected));
	}

	return (GP_OK);
}

static int
get_info_func (CameraFilesystem *fs, const char *folder, const char *file,
	       CameraFileInfo *info, void *data)
{
	Camera *camera = data;
	KonicaData *kd = camera->camlib_data;
	gulong image_id;
	guint information_buffer_size;
	guint exif_size;
	guchar *information_buffer = NULL;
	gboolean protected;

	gp_debug_printf (GP_DEBUG_LOW, "konica", "*** ENTER: get_info_func "
						 "***");

	CHECK (k_get_image_information (camera->port, kd->image_id_long, 
			gp_filesystem_number (camera->fs, folder, file), 
			&image_id, &exif_size, &protected,
			&information_buffer, &information_buffer_size));
	g_free (information_buffer);

	info->preview.fields = GP_FILE_INFO_SIZE | GP_FILE_INFO_TYPE;
	info->preview.size = information_buffer_size;
	strcpy (info->preview.type, GP_MIME_JPEG);

	info->file.fields = GP_FILE_INFO_SIZE | GP_FILE_INFO_PERMISSIONS |
			    GP_FILE_INFO_TYPE | GP_FILE_INFO_NAME;
	info->file.size = exif_size;
	info->file.permissions = GP_FILE_PERM_READ;
	if (!protected) info->file.permissions = GP_FILE_PERM_DELETE;
	strcpy (info->file.type, GP_MIME_JPEG);
	strcpy (info->file.name, file);

	return (GP_OK);
}

static int
camera_exit (Camera* camera)
{
        KonicaData *kd = camera->camlib_data;

	if (kd) {
		free (kd);
		kd = NULL;
	}

	return (GP_OK);
}

static int
camera_folder_delete_all (Camera* camera, const gchar* folder)
{
	unsigned int not_erased = 0;
	char tmp[1024];

	if (strcmp (folder, "/"))
		return (GP_ERROR_DIRECTORY_NOT_FOUND);

	CHECK (k_erase_all (camera->port, &not_erased));
	CHECK (gp_filesystem_format (camera->fs));

	if (not_erased) {
		sprintf (tmp, _("%i pictures could not be deleted because "
			 "they are protected!"), not_erased);
		gp_frontend_message (camera, tmp);
	}

	return (GP_OK);
}

static int 
get_file_func (CameraFilesystem *fs, const char *folder, const char *filename,
	       CameraFileType type, CameraFile *file, void *data)
{
	Camera *camera = data;
	gulong 		image_id;
	gchar*		image_id_string;
	KonicaData *kd = camera->camlib_data;
	unsigned char *fdata = NULL;
	long int size;

	if (strlen (filename) != 11)
		return (GP_ERROR_FILE_NOT_FOUND);
	if (!strcmp (filename, "??????.jpeg"))
		return (GP_ERROR_FILE_NOT_FOUND);
	if (strcmp (folder, "/"))
		return (GP_ERROR_DIRECTORY_NOT_FOUND);

	/* Check if we can get the image id from the filename. */
	image_id_string = g_strndup (filename, 6);
	image_id = atol (image_id_string);
	g_free (image_id_string);
	
	/* Get the image. */
	switch (type) {
	case GP_FILE_TYPE_PREVIEW:
		CHECK (k_get_image (camera->port, kd->image_id_long, image_id,
				    K_THUMBNAIL, (guchar **) &fdata,
				    (guint *) &size));
		break;
	case GP_FILE_TYPE_NORMAL:
		CHECK (k_get_image (camera->port, kd->image_id_long, image_id,
				    K_IMAGE_EXIF, (guchar **) &fdata,
				    (guint *) &size));
		break;
	default:
		return (GP_ERROR_NOT_SUPPORTED);
	}

	CHECK (gp_file_set_data_and_size (file, fdata, size));
	CHECK (gp_file_set_mime_type (file, GP_MIME_JPEG));

	return (GP_OK);
}

static int
camera_file_delete (Camera* camera, const gchar* folder, const gchar* filename)
{
	gchar*		tmp;
	gulong 		image_id; 
	KonicaData*	kd;

        gp_debug_printf (GP_DEBUG_LOW, "konica", "*** Entering camera_file_delete ***");
        g_return_val_if_fail (camera, 	GP_ERROR_BAD_PARAMETERS);
        g_return_val_if_fail (folder, 	GP_ERROR_BAD_PARAMETERS);
        g_return_val_if_fail (filename, GP_ERROR_BAD_PARAMETERS);

	if (strcmp (folder, "/")) return (GP_ERROR_DIRECTORY_NOT_FOUND);

	kd = (KonicaData *) camera->camlib_data;

	/* Check if we can get the image id from the filename. */
	g_return_val_if_fail (filename[0] != '?', GP_ERROR);
	tmp = g_strndup (filename, 6);
	image_id = atol (tmp);
	g_free (tmp);
	
	CHECK (k_erase_image (camera->port, kd->image_id_long, image_id));
	CHECK (gp_filesystem_delete (camera->fs, folder, filename));

	return (GP_OK);
}


static int 
camera_summary (Camera* camera, CameraText* summary)
{
	gchar*		model = NULL;
	gchar*		serial_number = NULL;
	guchar 		hardware_version_major;
	guchar 		hardware_version_minor;
	guchar 		software_version_major;
	guchar 		software_version_minor;
	guchar 		testing_software_version_major;
	guchar 		testing_software_version_minor;
	gchar*		name = NULL;
	gchar*		manufacturer = NULL;
        KonicaData*	kd;

	gp_debug_printf (GP_DEBUG_LOW, "konica", "*** ENTER: camera_summary "
			 "***");
        kd = (KonicaData *) camera->camlib_data;
	CHECK (k_get_information (camera->port, &model, &serial_number, 
		&hardware_version_major, &hardware_version_minor, 
		&software_version_major, &software_version_minor, 
		&testing_software_version_major,
		&testing_software_version_minor, &name, &manufacturer));

	sprintf (summary->text, 
		_("Model: %s\n"
		"Serial Number: %s,\n"
		"Hardware Version: %i.%i\n"
		"Software Version: %i.%i\n"
		"Testing Software Version: %i.%i\n"
		"Name: %s,\n"
		"Manufacturer: %s\n"),
		model, serial_number, hardware_version_major, 
		hardware_version_minor, software_version_major, 
		software_version_minor, testing_software_version_major, 
		testing_software_version_minor, name, manufacturer);

	return (GP_OK);
}

static int
camera_capture_preview (Camera* camera, CameraFile* file)
{
	unsigned char *data = NULL;
	long int size;

	CHECK (k_get_preview (camera->port, TRUE, &data, (guint*) &size));
	CHECK (gp_file_set_data_and_size (file, data, size));
	CHECK (gp_file_set_mime_type (file, GP_MIME_JPEG));

	return (GP_OK);
}

static int 
camera_capture (Camera* camera, gint type, CameraFilePath* path)
{
	KonicaData*	kd;
	gulong 		image_id;
	gint 		exif_size;
	guchar*		information_buffer = NULL;
	guint 		information_buffer_size;
	gboolean 	protected;
	gchar*		tmp;

        gp_debug_printf (GP_DEBUG_LOW, "konica", "*** Entering camera_capture ***");
	g_return_val_if_fail (camera, 	GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (type == GP_OPERATION_CAPTURE_IMAGE, GP_ERROR_NOT_SUPPORTED);
	g_return_val_if_fail (path,	GP_ERROR_BAD_PARAMETERS);

        kd = (KonicaData *) camera->camlib_data;

	/* Take the picture. */
	CHECK (k_take_picture (camera->port, kd->image_id_long, &image_id, 
		&exif_size, &information_buffer, &information_buffer_size, 
		&protected));
	
	g_free (information_buffer);
	tmp = g_strdup_printf ("%06i.jpeg", (gint) image_id);
	strcpy (path->name, tmp);
	g_free (tmp);
	strcpy (path->folder, "/");

	CHECK (gp_filesystem_append (camera->fs, path->folder, path->name));

	gp_debug_printf (GP_DEBUG_LOW, "konica",
			 "*** filename: %s", path->name);
	gp_debug_printf (GP_DEBUG_LOW, "konica",
			 "*** Leaving camera_capture ***");
	return (GP_OK);
}


static int 
camera_manual (Camera* camera, CameraText* manual)
{
	strcpy(manual->text, _("No manual available."));

	return (GP_OK);
}


static int 
camera_about (Camera* camera, CameraText* about) 
{
	g_return_val_if_fail (camera, 	GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (about, 	GP_ERROR_BAD_PARAMETERS);

	strcpy (about->text, 
		_("Konica library\n"
		"Lutz M�ller <urc8@rz.uni-karlsruhe.de>\n"
		"Support for all Konica and several HP cameras."));

	return (GP_OK);
}

static int
camera_get_config (Camera* camera, CameraWidget** window)
{
        CameraWidget *widget;
	CameraWidget *section;
	KStatus status;
	KPreferences preferences;
	int	year_4_digits;
	struct tm	tm_struct;
	time_t		t;
	float value_float;
	const char *name;
	GP_SYSTEM_DIR d;
	GP_SYSTEM_DIRENT de;

        gp_debug_printf (GP_DEBUG_LOW, "konica", "*** ENTER: "
			 "camera_get_config ***");

	/* Get the current settings. */
	CHECK (k_get_status (camera->port, &status));
	CHECK (k_get_preferences (camera->port, &preferences));

	/* Create the window. */
        gp_widget_new (GP_WIDGET_WINDOW, _("Konica Configuration"), window);

        /************************/
        /* Persistent Settings  */
        /************************/
	gp_widget_new (GP_WIDGET_SECTION, _("Persistent Settings"), &section);
        gp_widget_append (*window, section);

	/* Date */
	gp_widget_new (GP_WIDGET_DATE, _("Date and Time"), &widget);
	gp_widget_append (section, widget);
	if (status.date.year > 80) year_4_digits = status.date.year + 1900;
	else year_4_digits = status.date.year + 2000;
	tm_struct.tm_year = year_4_digits - 1900;
	tm_struct.tm_mon = status.date.month;
	tm_struct.tm_mday = status.date.day;
	tm_struct.tm_hour = status.date.hour;
	tm_struct.tm_min = status.date.minute;
	tm_struct.tm_sec = status.date.second;
	t = mktime (&tm_struct);
	gp_widget_set_value (widget, &t);

        /* Beep */
        gp_widget_new (GP_WIDGET_RADIO, _("Beep"), &widget);
        gp_widget_append (section, widget);
	gp_widget_add_choice (widget, _("On"));
	gp_widget_add_choice (widget, _("Off"));
        switch (preferences.beep) {
        case 0:
		gp_widget_set_value (widget, _("Off"));
		break;
        default:
		gp_widget_set_value (widget, _("On"));
                break;
        }
	gp_widget_set_info (widget, _("Shall the camera beep when taking a "
			    "picture?"));

        /* Self Timer Time */
        gp_widget_new (GP_WIDGET_RANGE, _("Self Timer Time"), &widget);
        gp_widget_append (section, widget);
        gp_widget_set_range (widget, 3, 40, 1);
	value_float = preferences.self_timer_time;
	gp_widget_set_value (widget, &value_float);

        /* Auto Off Time */
        gp_widget_new (GP_WIDGET_RANGE, _("Auto Off Time"), &widget);
        gp_widget_append (section, widget);
        gp_widget_set_range (widget, 1, 255, 1);
	value_float = preferences.shutoff_time;
	gp_widget_set_value (widget, &value_float);

        /* Slide Show Interval */
        gp_widget_new (GP_WIDGET_RANGE, _("Slide Show Interval"), &widget);
        gp_widget_append (section, widget);
        gp_widget_set_range (widget, 1, 30, 1);
	value_float = preferences.slide_show_interval;
	gp_widget_set_value (widget, &value_float);

        /* Resolution */
        gp_widget_new (GP_WIDGET_RADIO, _("Resolution"), &widget);
        gp_widget_append (section, widget);
        gp_widget_add_choice (widget, _("Low (576 x 436)"));
        gp_widget_add_choice (widget, _("Medium (1152 x 872)"));
        gp_widget_add_choice (widget, _("High (1152 x 872)"));
        switch (status.resolution) {
        case 1:
		gp_widget_set_value (widget, _("High (1152 x 872)"));
                break;
        case 3:
		gp_widget_set_value (widget, _("Low (576 x 436)")); 
                break;
        default:
		gp_widget_set_value (widget, _("Medium (1152 x 872)"));
                break;
        }

	/****************/
	/* Localization */
	/****************/
        gp_widget_new (GP_WIDGET_SECTION, _("Localization"), &section);
        gp_widget_append (*window, section);

        /* Language */
	d = GP_SYSTEM_OPENDIR (LOCALIZATION);
	if (d) {
	        gp_widget_new (GP_WIDGET_MENU, _("Language"), &widget);
	        gp_widget_append (section, widget);
		while ((de = GP_SYSTEM_READDIR (d))) {
			name = GP_SYSTEM_FILENAME (de);
			if (name && (*name != '.'))
				gp_widget_add_choice (widget, name);
		}
		gp_widget_set_value (widget, _("None selected"));
	}

	/* TV output format */
	gp_widget_new (GP_WIDGET_MENU, _("TV Output Format"), &widget);
	gp_widget_append (section, widget);
	gp_widget_add_choice (widget, _("NTSC"));
	gp_widget_add_choice (widget, _("PAL"));
	gp_widget_add_choice (widget, _("Do not display TV menu"));
	gp_widget_set_value (widget, _("None selected"));

	/* Date format */
	gp_widget_new (GP_WIDGET_MENU, _("Date Format"), &widget);
	gp_widget_append (section, widget);
	gp_widget_add_choice (widget, _("Month/Day/Year"));
	gp_widget_add_choice (widget, _("Day/Month/Year"));
	gp_widget_add_choice (widget, _("Year/Month/Day"));
	gp_widget_set_value (widget, _("None selected"));

        /********************************/
        /* Session-persistent Settings  */
        /********************************/
        gp_widget_new (GP_WIDGET_SECTION, _("Session-persistent Settings"),
		       &section);
        gp_widget_append (*window, section);

        /* Flash */
        gp_widget_new (GP_WIDGET_RADIO, _("Flash"), &widget);
        gp_widget_append (section, widget);
        gp_widget_add_choice (widget, _("Off"));
        gp_widget_add_choice (widget, _("On"));
        gp_widget_add_choice (widget, _("On, red-eye reduction"));
        gp_widget_add_choice (widget, _("Auto"));
        gp_widget_add_choice (widget, _("Auto, red-eye reduction"));
        switch (status.flash) {
        case 0:
		gp_widget_set_value (widget, _("Off"));
                break;
        case 1:
		gp_widget_set_value (widget, _("On"));
                break;
        case 5:
		gp_widget_set_value (widget, _("On, red-eye reduction"));
                break;
        case 6:
		gp_widget_set_value (widget, _("Auto, red-eye reduction"));
                break;
        default:
		gp_widget_set_value (widget, _("Auto"));
                break;
        }

        /* Exposure */
        gp_widget_new (GP_WIDGET_RANGE, _("Exposure"), &widget);
        gp_widget_append (section, widget);
        gp_widget_set_range (widget, 0, 255, 1);
	value_float = status.exposure;
	gp_widget_set_value (widget, &value_float);

        /* Focus */
        gp_widget_new (GP_WIDGET_RADIO, _("Focus"), &widget);
        gp_widget_append (section, widget);
        gp_widget_add_choice (widget, _("Fixed"));
        gp_widget_add_choice (widget, _("Auto"));
        switch ((guint) (status.focus / 2)) {
        case 1:
		gp_widget_set_value (widget, _("Auto"));
                break;
        default:
		gp_widget_set_value (widget, _("Fixed"));
                break;
        }

        /************************/
        /* Volatile Settings    */
        /************************/
        gp_widget_new (GP_WIDGET_SECTION, _("Volatile Settings"), &section);
        gp_widget_append (*window, section);

        /* Self Timer */
        gp_widget_new (GP_WIDGET_RADIO, _("Self Timer"), &widget);
        gp_widget_append (section, widget);
        gp_widget_add_choice (widget, _("Self Timer (only next picture)"));
        gp_widget_add_choice (widget, _("Normal"));
        switch (status.focus % 2) {
        case 1:
		gp_widget_set_value (widget, _("Self Timer ("
				     "next picture only)"));
                break;
        default:
		gp_widget_set_value (widget, _("Normal"));
                break;
        }

	/* That's it. */
	return (GP_OK);
}
 
static int
camera_set_config (Camera *camera, CameraWidget *window)
{
	CameraWidget *section;
	CameraWidget *widget_focus;
	CameraWidget *widget_self_timer;
	CameraWidget *widget;
	KDate date;
	KDateFormat date_format = K_DATE_FORMAT_YEAR_MONTH_DAY;
	KTVOutputFormat tv_output_format = K_TV_OUTPUT_FORMAT_HIDE;
	guint		beep = 0;
        gint 		j = 0;
        unsigned char  *data;
        long int	data_size;
	guchar 		focus_self_timer = 0;
	gint		i;
	gfloat		f;
	gchar*		c;
	struct tm*	tm_struct;
	int		result;

        gp_debug_printf (GP_DEBUG_LOW, "konica", "*** ENTER: "
			 "camera_set_config ***");

        /************************/
        /* Persistent Settings  */
        /************************/
	gp_widget_get_child_by_label (window, _("Persistent Settings"),
				      &section);

	/* Date & Time */
	gp_widget_get_child_by_label (section, _("Date and Time"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &i);
		tm_struct = localtime ((time_t*) &i);
		date.year   = tm_struct->tm_year - 100;
		date.month  = tm_struct->tm_mon;
		date.day    = tm_struct->tm_mday;
		date.hour   = tm_struct->tm_hour;
		date.minute = tm_struct->tm_min;
		date.second = tm_struct->tm_sec;
		CHECK (k_set_date_and_time (camera->port, date));
	}

	/* Beep */
	gp_widget_get_child_by_label (section, _("Beep"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &c);
		if (strcmp (c, _("Off")) == 0)
			beep = 0;
		else
			beep = 1;
		CHECK (k_set_preference (camera->port, K_PREFERENCE_BEEP,
					 beep));
	}

	/* Self Timer Time */
	gp_widget_get_child_by_label (section, _("Self Timer Time"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &f);
		CHECK (k_set_preference (camera->port,
			K_PREFERENCE_SELF_TIMER_TIME, (gint) f));
	}

	/* Auto Off Time */
	gp_widget_get_child_by_label (section, _("Auto Off Time"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &f);
		CHECK (k_set_preference (camera->port,
					K_PREFERENCE_AUTO_OFF_TIME, (int) f));
	}

	/* Slide Show Interval */
	gp_widget_get_child_by_label (section, _("Slide Show Interval"),
				      &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &f);
		CHECK (k_set_preference (camera->port,
				K_PREFERENCE_SLIDE_SHOW_INTERVAL, (gint) f));
	}

	/* Resolution */
	gp_widget_get_child_by_label (section, _("Resolution"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &c);
		if (!strcmp (c, _("High (1152 x 872)")))
			j = 1;
                else if (!strcmp (c, _("Low (576 x 436)")))
			j = 3;
                else
			j = 0;
        	CHECK (k_set_preference (camera->port,
					K_PREFERENCE_RESOLUTION, j));
	}

        /****************/
        /* Localization */
        /****************/
	gp_widget_get_child_by_label (window, _("Localization"), &section);

	/* Localization File */
	gp_widget_get_child_by_label (section, _("Language"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &c);
		if (strcmp (c, _("None selected"))) {
	                data = NULL;
			data_size = 0;

			/* Read localization file */
			result = localization_file_read (camera, c,
							 &data, &data_size);
			if (result != GP_OK) {
				g_free (data);
				return (result);
			}

			/* Go! */
			result = k_localization_data_put (camera->port,
							  data, data_size);
			g_free (data);
			CHECK (result);
		}
	}

	/* TV Output Format */
	gp_widget_get_child_by_label (section, _("TV Output Format"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &c);
		if (strcmp (c, _("None selected"))) {
			if (!strcmp (c, _("NTSC")))
				tv_output_format = K_TV_OUTPUT_FORMAT_NTSC;
			else if (!strcmp (c, _("PAL")))
				tv_output_format = K_TV_OUTPUT_FORMAT_PAL;
			else if (!strcmp (c, _("Do not display TV menu")))
				tv_output_format = K_TV_OUTPUT_FORMAT_HIDE;
			else
				g_assert_not_reached ();
			CHECK (k_localization_tv_output_format_set (
					camera->port, tv_output_format));
		}
	}

	/* Date Format */
	gp_widget_get_child_by_label (section, _("Date Format"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &c);
		if (strcmp (c, _("None selected"))) {
			if (!strcmp (c, _("Month/Day/Year")))
				date_format = K_DATE_FORMAT_MONTH_DAY_YEAR;
	                else if (!strcmp (c, _("Day/Month/Year")))
				date_format = K_DATE_FORMAT_DAY_MONTH_YEAR;
			else if (!strcmp (c, _("Year/Month/Day")))
				date_format = K_DATE_FORMAT_YEAR_MONTH_DAY;
			else 
				g_assert_not_reached ();
			CHECK (k_localization_date_format_set (camera->port, 
						       date_format));
		}
	}

        /********************************/
        /* Session-persistent Settings  */
        /********************************/
	gp_widget_get_child_by_label (window, _("Session-persistent Settings"),
				      &section);

	/* Flash */
	gp_widget_get_child_by_label (section, _("Flash"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &c);
		if (!strcmp (c, _("Off")))
			j = 0;
		else if (!strcmp (c, _("On")))
			j = 1;
                else if (!strcmp (c, _("On, red-eye reduction")))
			j = 5;
		else if (!strcmp (c, _("Auto")))
			j = 2;
		else
			j = 6;
		CHECK (k_set_preference (camera->port, K_PREFERENCE_FLASH, j));
	}

	/* Exposure */
	gp_widget_get_child_by_label (section, _("Exposure"), &widget);
	if (gp_widget_changed (widget)) {
		gp_widget_get_value (widget, &f);
		CHECK (k_set_preference (camera->port,
					K_PREFERENCE_EXPOSURE, (gint) f));
	}

	/* Focus will be set together with self timer. */
	gp_widget_get_child_by_label (section, _("Focus"), &widget_focus);

        /************************/
        /* Volatile Settings    */
        /************************/
	gp_widget_get_child_by_label (window, _("Volatile Settings"), &section);

	/* Self Timer (and Focus) */
	gp_widget_get_child_by_label (section, _("Self Timer"),
				      &widget_self_timer);
	if (gp_widget_changed (widget_focus) &&
	    gp_widget_changed (widget_self_timer)) {
		gp_widget_get_value (widget_focus, &c);
		if (!strcmp (c, _("Auto")))
			focus_self_timer = 2;
		else
			focus_self_timer = 0;
		gp_widget_get_value (widget_self_timer, &c);
		if (!strcmp (c, _("Self Timer (only next picture)")))
			focus_self_timer++;
		CHECK (k_set_preference (camera->port,
			K_PREFERENCE_FOCUS_SELF_TIMER, focus_self_timer));
	}

	/* We are done. */
	return (GP_OK);
}

static gchar *
camera_result_as_string (Camera* camera, gint result)
{
	/* Really an error? */
	g_return_val_if_fail (result < 0, _("Unknown error"));

	/* libgphoto2 error? */
	if (-result < 1000) return (gp_result_as_string (result));

	/* Our error? */
	if ((0 - result - 1000) <
		(guint) (sizeof (konica_results) / sizeof (*konica_results)))
		return _(konica_results [0 - result - 1000]);
	
	return _("Unknown error");
}


int
localization_file_read (Camera *camera, const char *file_name,
			unsigned char **data, long int *data_size)
{
	FILE *file;
	gulong j;
	gchar f;
	guchar c[] = "\0\0";
	gulong line_number;
	guchar checksum;
	gulong fcs;
	guint d;
	char path[1024];

	strcpy (path, LOCALIZATION);
	strcat (path, "/");
	strcat (path, file_name);
	file = fopen (path, "r");
	if (!file)
		return (GP_ERROR_FILE_NOT_FOUND);

	/* Allocate the memory */
	*data_size = 0;
	*data = malloc (sizeof (char) * 65536);
	if (!*data)
		return (GP_ERROR_NO_MEMORY);

	j = 0; 
	line_number = 1;
        do {
                f = fgetc (file);
                switch (f) {
                case '\n': 
			line_number++;
			continue;
		case EOF:
			break;
                case '#':
			/************************/
                        /* Comment: Discard.    */
			/************************/
                        do {
                                f = fgetc (file);
                        } while ((f != '\n') && (f != EOF)); 
			if (f == '\n') line_number++;
                        continue;
                case '\t': 
			continue;
                case ' ': 
			continue;
                default: 
                        /****************************************/
                        /* j == 0: We have to read the second   */
                        /*         half of the byte to send.    */
                        /* j == 1: We'll compose our byte.      */
                        /****************************************/
			if ((f != '0') && (f != '1') && (f != '2') &&
			    (f != '3') && (f != '4') && (f != '5') &&
			    (f != '6') && (f != '7') && (f != '8') &&
			    (f != '9') && (f != 'A') && (f != 'B') &&
			    (f != 'C') && (f != 'D') && (f != 'E') &&
			    (f != 'F')) {
				gp_debug_printf (GP_DEBUG_LOW, "konica",
						 "Error in localization "
						 "file: '%c' in line %i is "
						 "not allowed.", f,
						 (int) line_number);
				fclose (file);
				return (GP_ERROR_CORRUPTED_DATA);
			}
			c[j] = f;
			if (j == 1) {
				if (sscanf (&c[0], "%X", &d) != 1) {
					gp_debug_printf (GP_DEBUG_LOW,
							 "konica", "Error in "
							 "localization file.");
					return (GP_ERROR_CORRUPTED_DATA);
				}
				(*data)[*data_size] = d;
				(*data_size)++;
				if (*data_size == 65536) {
					gp_frontend_message (
						camera, 
						_("Localization file too long!"));
					fclose (file);
					return (FALSE);
				}
			}
			j = 1 - j;
			continue;
                } 
	} while (f != EOF);
	fclose (file);
	/*********************************/
	/* Calculate and check checksum. */
	/*********************************/
	checksum = 0;
	g_warning (_("Checksum not implemented!"));
	/*FIXME: There's a checksum at (*data)[100]. I could not
	  figure out how it is calculated. */
	/*********************************************/
	/* Calculate and check frame check sequence. */
	/*********************************************/
	fcs = 0;
	g_warning (_("Frame check sequence not implemented!"));
	/* FIXME: There's a frame check sequence at (*data)[108]
	   and (*data)[109]. I could not figure out how it is
	   calculated. */
	gp_debug_printf (GP_DEBUG_LOW, "konica", "-> %i bytes read.\n", (gint) *data_size);
	return (TRUE);
}

int 
camera_init (Camera* camera)
{
        gint i;
        gboolean image_id_long;
	int inep, outep;
        KonicaData *kd;
        gp_port_settings settings;

        g_return_val_if_fail (camera, GP_ERROR_BAD_PARAMETERS);

        /* First, set up all the function pointers. */
        camera->functions->exit                 = camera_exit;
        camera->functions->folder_delete_all    = camera_folder_delete_all;
        camera->functions->file_delete          = camera_file_delete;
        camera->functions->get_config           = camera_get_config;
        camera->functions->set_config           = camera_set_config;
        camera->functions->capture              = camera_capture;
        camera->functions->capture_preview      = camera_capture_preview;
        camera->functions->summary              = camera_summary;
        camera->functions->manual               = camera_manual;
        camera->functions->about                = camera_about;
        camera->functions->result_as_string     = camera_result_as_string;

        /* Lookup model information */
        for (i = 0; konica_cameras [i].model; i++)
                if (!strcmp (konica_cameras [i].model, camera->model))
                        break;
        if (!konica_cameras [i].model)
                return (GP_ERROR_MODEL_NOT_FOUND);
        image_id_long = konica_cameras [i].image_id_long;
        inep          = konica_cameras [i].inep;
        outep         = konica_cameras [i].outep;

        /* Initiate the connection */
        switch (camera->port->type) {
        case GP_PORT_SERIAL:

                /* Set up the device */
                strcpy (settings.serial.port, camera->port_info->path);
                settings.serial.bits = 8;
                settings.serial.parity = 0;
                settings.serial.stopbits = 1;
                CHECK (gp_port_settings_set (camera->port, settings));

                /* Initiate the serial connection */
                CHECK (init_serial_connection (camera));

                break;
        case GP_PORT_USB:

                /* Set up the device */
                settings.usb.inep      = inep;
                settings.usb.outep     = outep;
                settings.usb.config    = 1;
                settings.usb.interface = 0;
                settings.usb.altsetting = 0;
                CHECK (gp_port_settings_set (camera->port, settings));

                /* Initiate the connection */
                CHECK (k_init (camera->port));

                break;
        default:
                return (GP_ERROR_IO_UNKNOWN_PORT);
        }

        /* Store some data we constantly need. */
        kd = g_new (KonicaData, 1);
        camera->camlib_data = kd;
        kd->image_id_long = image_id_long;

        /* Set up the filesystem */
        gp_filesystem_set_info_funcs (camera->fs, get_info_func,
                                      set_info_func, camera);
        gp_filesystem_set_list_funcs (camera->fs, file_list_func,
                                      NULL, camera);
	gp_filesystem_set_file_func (camera->fs, get_file_func, camera);

        gp_debug_printf (GP_DEBUG_LOW, "konica", "*** EXIT: "
                         "camera_init ***");

        return (GP_OK);
}

