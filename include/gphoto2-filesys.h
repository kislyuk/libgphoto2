/* gphoto2-filesys.h: Filesystem emulation for cameras that don't support 
 *                    filenames. In addition, it can be used to cache the
 *                    contents of the camera in order to avoid traffic.
 *
 * Copyright (C) 2000 Scott Fritzinger
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

#ifndef __GPHOTO2_FILESYS_H__
#define __GPHOTO2_FILESYS_H__

#include <gphoto2-list.h>
#include <gphoto2-camera.h>

/* You don't really want to know what's inside, do you? */
typedef struct _CameraFilesystem CameraFilesystem;

int gp_filesystem_new	 (CameraFilesystem **fs);
int gp_filesystem_free	 (CameraFilesystem *fs);

int gp_filesystem_folder_set_dirty (CameraFilesystem *fs, const char *folder,
				    int dirty);
int gp_filesystem_folder_is_dirty  (CameraFilesystem *fs, const char *folder);

/* Adding */
int gp_filesystem_populate (CameraFilesystem *fs, const char *folder,
			    char *format, int count);
int gp_filesystem_append   (CameraFilesystem *fs, const char *folder,
			    const char *filename);

/* Deleting */
int gp_filesystem_format     (CameraFilesystem *fs);
int gp_filesystem_delete     (CameraFilesystem *fs, const char *folder,
			      const char *filename);
int gp_filesystem_delete_all (CameraFilesystem *fs, const char *folder);

/* Information retreival */
int gp_filesystem_count	       (CameraFilesystem *fs, const char *folder);
int gp_filesystem_name         (CameraFilesystem *fs, const char *folder,
			        int filenumber, const char **filename);
int gp_filesystem_number       (CameraFilesystem *fs, const char *folder,
			        const char *filename);
int gp_filesystem_get_folder   (CameraFilesystem *fs, const char *filename,
			        const char **folder);
int gp_filesystem_list_files   (CameraFilesystem *fs, const char *folder,
				CameraList *list);
int gp_filesystem_list_folders (CameraFilesystem *fs, const char *folder,
				CameraList *list);

/* File information */
typedef int (*CameraFilesystemInfoFunc) (CameraFilesystem *fs,
					 const char *folder,
					 const char *filename,
					 CameraFileInfo *info, void *data);
int gp_filesystem_set_info_funcs (CameraFilesystem *fs,
				  CameraFilesystemInfoFunc get_info_func,
				  CameraFilesystemInfoFunc set_info_func,
				  void *data);
int gp_filesystem_get_info       (CameraFilesystem *fs, const char *folder,
				  const char *filename, CameraFileInfo *info);
int gp_filesystem_set_info       (CameraFilesystem *fs, const char *folder,
				  const char *filename, CameraFileInfo *info);

int gp_filesystem_dump         (CameraFilesystem *fs);

#endif /* __GPHOTO2_FILESYS_H__ */
