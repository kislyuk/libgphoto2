#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gphoto2.h>

#include "globals.h"

CameraFile* gp_file_new () {

    CameraFile *file;

    file = (CameraFile*)malloc(sizeof(CameraFile));
    strcpy(file->type, "unknown/unknown");
    strcpy(file->name, "");
    file->data = NULL;
    file->size = 0;
    file->bytes_read = 0;
    file->session = glob_session_file++;
    file->ref_count = 1;

    return(file);
}

int gp_file_free (CameraFile *file) {

    gp_file_clean(file);
    free(file);
    return(GP_OK);
}

int gp_file_ref (CameraFile *file) {

    file->ref_count += 1;

    return (GP_OK);
}

int gp_file_unref (CameraFile *file) {

    if (file == NULL)
        return GP_ERROR_BAD_PARAMETERS;

    file->ref_count -= 1;

    if (file->ref_count == 0)
        gp_file_free(file);

    return (GP_OK);
}

int gp_file_session (CameraFile *file) {

        return (file->session);
}

int gp_file_append (CameraFile *file, char *data, int size) {

        char *t;

        if (size < 0)
                return (GP_ERROR_BAD_PARAMETERS);

        if (!file->data) {
                file->data = (char*)malloc(sizeof(char) * (size));
        } else {
               t = (char*)realloc(file->data, sizeof(char) * (file->size + size));
               if (!t)
                   return GP_ERROR_NO_MEMORY;
               file->data = t;
        }
        memcpy(&file->data[file->size], data, size);

        file->bytes_read = size;
        file->size += size;

        return (GP_OK);
}

int gp_file_get_last_chunk (CameraFile *file, char **data, int *size) {

        if (file->bytes_read == 0) {
                /* chunk_add was never called. return safely. */
                *data = NULL;
                *size = 0;
                return (GP_ERROR);
        }

        /* They must free the returned data! */
        *data = (char*)malloc(file->bytes_read);
        memcpy(*data, &file->data[file->size - file->bytes_read], file->bytes_read);
        *size = file->bytes_read;

        return (GP_OK);
}

int gp_file_save (CameraFile *file, char *filename) {

        FILE *fp;

        if ((fp = fopen(filename, "wb"))==NULL)
                return (GP_ERROR);
        fwrite(file->data, (size_t)sizeof(char), (size_t)file->size, fp);
        fclose(fp);

        return (GP_OK);
}

int gp_file_open (CameraFile *file, char *filename) {

        FILE *fp;
        char *name, *dot;
        long size, size_read;
	int  i;
	/*
	 * mime types that cannot be determined by the filename
	 * extension. Better hack would be to use library that examine
	 * file content instead, like gnome-vfs mime handling, or 
	 * gnome-mime, whatever.
	 */
	static char *mime_table[] = {
	    "jpg",  "jpeg",
	    "tif",  "tiff",
	    "ppm",  "x-portable-pixmap",
	    "pgm",  "x-portable-graymap",
	    "pbm",  "x-portable-bitmap",
	    "png",  "x-png",
	    NULL};

        gp_file_clean(file);

        fp = fopen(filename, "r");
        if (!fp)
                return (GP_ERROR);
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);

        file->data = (char*)malloc(sizeof(char)*(size + 1));
        if (!file->data)
                return (GP_ERROR_BAD_PARAMETERS);
        size_read = fread(file->data, (size_t)sizeof(char), (size_t)size, fp);
        if (ferror(fp)) {
                gp_file_clean(file);
                return (GP_ERROR);
        }
        fclose(fp);

        file->size = size_read;
        file->data[size_read] = 0;

        name = strrchr(filename, '/');
        if (name)
                strcpy(file->name, name + 1);
           else
                strcpy(file->name, filename);

	/* MIME lookup */
	dot = strrchr(filename, '.');
	if (dot) {
	    for (i = 0; mime_table[i] ; i+=2)
		if (!strcasecmp (mime_table[i], dot+1)) {
		    sprintf (file->type,"image/%s", mime_table[i+1]);
		    break;
		}
	    if (!mime_table[i])
		/* 
		 * We did not found the type in the lookup table,
		 * so we use the file suffix as mime type.
		 */
		sprintf(file->type, "image/%s", dot + 1);
	} else
	    /*
	     * Damn, no filename suffix...
	     */
	    strcpy(file->type, "image/unknown");

        return (GP_OK);
}

int gp_file_clean (CameraFile *file) {

        /*
        frees a CameraFile's components, not the CameraFile itself.
        this is used to prep a CameraFile struct to be filled.
        */
        if (file->data != NULL)
                free(file->data);
        strcpy(file->name, "");
        file->data = NULL;
        file->size = 0;
        file->bytes_read = 0;
        return(GP_OK);
}
