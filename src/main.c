/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Kevin DeKorte 2007 <kdekorte@gmail.com>
 * 
 * main.c is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * main.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with main.c.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gstrfuncs.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>
#include "left.h"
#include "right.h"
#include "../config.h"
#include <signal.h>
#include <unistd.h>

static gboolean verbose = FALSE;
static GOptionEntry entries[] = {
    {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Show more ouput on the console", NULL},
    {NULL}
};
    
int main(gint argc, gchar * argv[])
{

    GError *error;
    GdkPixbuf *pb_prescale;
    GdkPixbuf *pb;
    GdkPixbuf *pb_left;
    GdkPixbuf *pb_right;
    GdkPixbuf *output;
    GdkPixbuf *scaled;
    gchar *dirname = NULL;
    gchar *filename = NULL;
    gint exit_status;
    gchar *stderr;
    gchar *stdout;
    GRand *rand;
    gboolean saved = FALSE;
    gint size;
    char *av1[255];
    gint ac1 = 0;
    char *av2[255];
    gint ac2 = 0;
    char *av3[255];
    gint ac3 = 0;
    gint count = 0;
    gint length = 0;
    gchar *id_length;
    GOptionContext *context;
    time_t starttime;
    GPid pid;
    GSpawnFlags flags;
    gint fhout;
    gchar buffer[1024];
    gint startsecond;
    struct stat buf1;
    struct stat buf2;
        
    g_type_init();
    error = NULL;
    pb_left = gdk_pixbuf_new_from_inline(-1, left, FALSE, NULL);
    pb_right = gdk_pixbuf_new_from_inline(-1, right, FALSE, NULL);
    
    context = g_option_context_new("[input] [output] {size}\n\n\tThumbnail generator using mplayer, version " VERSION);
    g_option_context_add_main_entries(context, entries, NULL);
    g_option_context_parse(context, &argc, &argv, &error);
    if (error != NULL) {
        g_error_free(error);
        error = NULL;
    }
    
    if (!g_file_test(argv[1], G_FILE_TEST_EXISTS)) {
    	g_printf("File not found\n");
    	return(1);
    }
    
    av1[ac1++] = g_strdup_printf("file");
    av1[ac1++] = g_strdup_printf("--brief");
    av1[ac1++] = g_strdup_printf("%s",argv[1]);
    av1[ac1] = NULL;
    
    
    g_spawn_sync(NULL,av1,NULL,G_SPAWN_SEARCH_PATH, NULL, NULL, &stdout, &stderr, &exit_status, &error);
    if (error != NULL) {
        printf("Error when running: %s\n",error->message);
        g_error_free(error);
        error = NULL;
        return(1);
    }
    
    if (g_ascii_strcasecmp(stdout,"data") == 0) {
        printf("Input is not a media file\n");
        return(1);
    }
        
    pb = gdk_pixbuf_new_from_file(argv[1], &error);

    rand = g_rand_new();


    if (error != NULL) {
        g_error_free(error);
        error = NULL;

            av3[ac3++] = g_strdup_printf("mplayer");
            av3[ac3++] = g_strdup_printf("-identify");
            av3[ac3++] = g_strdup_printf("-ao");
            av3[ac3++] = g_strdup_printf("null");
            av3[ac3++] = g_strdup_printf("-vo");
            av3[ac3++] = g_strdup_printf("null");
            av3[ac3++] = g_strdup_printf("-frames");
            av3[ac3++] = g_strdup_printf("0");
            av3[ac3++] = g_strdup_printf("%s",argv[1]);
            av3[ac3] = NULL;
        
            if (verbose) {
                ac3 = 0;
                while(av3[ac3] != NULL) {
                    printf("%s ",av3[ac3++]);
                }
                printf("\n");
            }
	    
	    g_spawn_async_with_pipes(NULL,av3, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &pid, NULL, &fhout, NULL, &error);

            if (error != NULL) {
                printf("Error when running: %s\n",error->message);
                g_error_free(error);
                error = NULL;
                return(1);
            }        
            
	    time(&starttime);
	    stdout = g_strdup_printf(" ");
	    while(difftime(time(NULL),starttime) < 2) {
		memset(buffer,0,1024);
		if (read(fhout,buffer,1024) > 0) {
			stdout = g_strconcat(stdout,buffer,NULL);
		} else {
			break;
		}
	    }
	    close(fhout);
		
	    if (difftime(time(NULL),starttime) > 2)	
	    	kill(pid,9);

            if (verbose) {
                printf("%s\n",stdout);   
                printf("ERRORS *********\n");
                printf("%s\n",stderr);   
            }
            
            id_length = g_strrstr(stdout,"ID_LENGTH");
	    if (id_length != NULL)
            	length = (gint) g_ascii_strtod(id_length+strlen("ID_LENGTH="), NULL) - 2;
            if (length <= 0)
                length = 1; 
            if (length > 600)
                length = 600; 

        
            if (g_strrstr(stdout,"ID_VIDEO_FORMAT") == NULL) {
                printf("non-video file, no thumbnail\n");
                return(1);
            }
            
            dirname = g_strdup_printf("%s", tempnam("/tmp", "nailerXXXXXX"));
            filename = g_strdup_printf("%s/00000003.jpg", dirname);

            // run mplayer and try to get the first frame and convert it to a jpeg
	    while (count < 10 && !g_file_test(filename, G_FILE_TEST_EXISTS)) {
		ac2 = 0;
		// yes I know there is a potential memory leak here ;)
		av2[ac2++] = g_strdup_printf("mplayer");
		av2[ac2++] = g_strdup_printf("-nosound");
		av2[ac2++] = g_strdup_printf("-vo");
		av2[ac2++] = g_strdup_printf("jpeg:outdir=%s",dirname);
		av2[ac2++] = g_strdup_printf("-ss");
		startsecond = (int)(length * ((10 * (count + 1)) / 100.0));
		av2[ac2++] = g_strdup_printf("%i",startsecond);
		av2[ac2++] = g_strdup_printf("-frames");
		av2[ac2++] = g_strdup_printf("3");
		av2[ac2++] = g_strdup_printf("%s",argv[1]);
		av2[ac2] = NULL;
		
		if (verbose) {
			printf("\n");
			ac2 = 0;
			while(av2[ac2] != NULL) {
				printf("%s ",av2[ac2++]);
			}
			printf("\n");
		}
	
		flags = G_SPAWN_SEARCH_PATH;
		if (!verbose) {
			flags = flags | G_SPAWN_STDOUT_TO_DEV_NULL |  G_SPAWN_STDERR_TO_DEV_NULL;
		}
		g_spawn_async(NULL,av2,NULL,flags, NULL, NULL, &pid, &error);
	    	time(&starttime);

		while(difftime(time(NULL),starttime) < 15) {
			if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
				while(1) {
					g_stat(filename, &buf1);
					g_usleep(100000);
					g_stat(filename, &buf2);
					if (buf1.st_mtime == buf2.st_mtime)
						break;
					if (verbose)
						printf("file exists, but still being modified\n");
				}
				pb_prescale = gdk_pixbuf_new_from_file(filename, &error);
				if (error != NULL) {
					if (verbose)
						printf("file exists, but not valid\n");
					g_error_free(error);
					error = NULL;
				} else {
					break;
				}
			}
			g_usleep(100000);
		}

		if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
			kill(pid,9);
		}

		if (error != NULL) {
			printf("Error when running: %s\n",error->message);
			g_error_free(error);
			error = NULL;
			return(1);
		}
		
		if (verbose) {
			printf("%s\n",stdout);   
			printf("ERRORS *********\n");
			printf("%s\n",stderr);   
		}
		count++;
	    }
	    if (difftime(time(NULL),starttime) > 15) {
		saved = FALSE;
	    }            

            if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
                pb_prescale = gdk_pixbuf_new_from_file(filename, &error);
                if (error != NULL) {
		    if (verbose)
                        printf("Error when loading pixbuf: %s\n",error->message);
                    g_error_free(error);
                    error = NULL;
                } else {
		    if (verbose) {
			printf("mplayer image size is %i x %i\n",
				gdk_pixbuf_get_width(pb_prescale),gdk_pixbuf_get_height(pb_prescale));
			printf("scaling to %i x %i\n",  
				238 * gdk_pixbuf_get_width(pb_prescale) / gdk_pixbuf_get_height(pb_prescale),238);
		    }

                    pb = gdk_pixbuf_scale_simple(pb_prescale,
                                                     238 * gdk_pixbuf_get_width(pb_prescale) / gdk_pixbuf_get_height(pb_prescale),
                                                     238,
                                                     GDK_INTERP_BILINEAR);

                    output = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(pb),
                                            gdk_pixbuf_get_has_alpha(pb),
                                            gdk_pixbuf_get_bits_per_sample(pb),
                                            gdk_pixbuf_get_width(pb_left) +
                                            gdk_pixbuf_get_width(pb) +
                                            gdk_pixbuf_get_width(pb_right),
                                            gdk_pixbuf_get_height(pb));

                    gdk_pixbuf_copy_area(pb_left,
                                         0,
                                         0,
                                         gdk_pixbuf_get_width(pb_left),
                                         gdk_pixbuf_get_height(pb_left), output, 0, 0);

                    gdk_pixbuf_copy_area(pb,
                                         0,
                                         0,
                                         gdk_pixbuf_get_width(pb),
                                         gdk_pixbuf_get_height(pb),
                                         output, gdk_pixbuf_get_width(pb_left), 0);

                    gdk_pixbuf_copy_area(pb_right,
                                         0,
                                         0,
                                         gdk_pixbuf_get_width(pb_right),
                                         gdk_pixbuf_get_height(pb_right),
                                         output,
                                         gdk_pixbuf_get_width(pb_left) + gdk_pixbuf_get_width(pb),
                                         0);

                    if (argc < 4) {
                        size = 128;
                    } else {
                        size = (gint) g_ascii_strtod(argv[3], NULL);
			if (size < 1) {
				size = 128;
			}
                    }

		    if (verbose) {
			printf("bordered image size is %i x %i\n",
				gdk_pixbuf_get_width(output),gdk_pixbuf_get_height(output));
			printf("scaling to %i x %i\n", 
				size, size * gdk_pixbuf_get_height(output) / gdk_pixbuf_get_width(output));
		    }

                    scaled = gdk_pixbuf_scale_simple(output,
                                                     size,
                                                     size * gdk_pixbuf_get_height(output) /
                                                     gdk_pixbuf_get_width(output),
                                                     GDK_INTERP_BILINEAR);

		    if (g_strrstr(argv[2],".jpeg") != NULL || g_strrstr(argv[2],".jpg") != NULL){
                        gdk_pixbuf_save(scaled, argv[2], "jpeg", &error, "quality", "100", NULL);
                    	saved = TRUE;
                    } else {
                        gdk_pixbuf_save(scaled, argv[2], "png", &error, NULL);
                    	saved = TRUE;
                    } 
                    if (error != NULL) {
                	printf("Error while saving: %s\n",error->message);
                        g_error_free(error);
                        error = NULL;
			saved = FALSE;
                    }

		    if (verbose) {
			printf("saved as %s\n",argv[2]);
		    }

                }
            } else {
		return 1;
	    }
            if (filename != NULL) {
                g_remove(filename);
                g_free(filename);
                filename = g_strdup_printf("%s/00000002.jpg", dirname);
                g_remove(filename);
                g_free(filename);
                filename = g_strdup_printf("%s/00000001.jpg", dirname);
                g_remove(filename);
                g_free(filename);
                
            }

            if (dirname != NULL) {
                g_remove(dirname);
                g_free(dirname);
            }
    } else {
        if (argc < 4) {
            size = 128;
        } else {
            size = (gint) g_ascii_strtod(argv[3], NULL);
	    if (size < 1) {
		size = 128;
	    }
        }

	if (verbose) {
		printf("Loading image directly\n");
		printf("image size is %i x %i\n",gdk_pixbuf_get_width(pb),gdk_pixbuf_get_height(pb));
		printf("scaling to %i x %i\n", size, size * gdk_pixbuf_get_height(pb) / gdk_pixbuf_get_width(pb));
	}
        
        scaled = gdk_pixbuf_scale_simple(pb,
                                         size,
                                         size * gdk_pixbuf_get_height(pb) /
                                                 gdk_pixbuf_get_width(pb),
                                                 GDK_INTERP_BILINEAR);
        
	if (g_strrstr(argv[2],".jpeg") != NULL || g_strrstr(argv[2],".jpg") != NULL){
		gdk_pixbuf_save(scaled, argv[2], "jpeg", &error, "quality", "100", NULL);
		saved = TRUE;
	} else { 
		gdk_pixbuf_save(scaled, argv[2], "png", &error, NULL);
		saved = TRUE;
	} 

	if (error != NULL) {
		printf("Error while saving: %s\n",error->message);
		g_error_free(error);
		error = NULL;
		saved = FALSE;
	}

    }

    g_rand_free(rand);
    
    if (saved) {
    	return (0);
    } else {
	return (1);
    }

}
