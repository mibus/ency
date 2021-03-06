/*****************************************************************************/
/* star trek ency reader: Reads the Star Trek Encyclopedia (1998 version)    */
/* Also reads the various Omnipedias & Episode guides                        */
/* Copyright (C) 1998 Robert Mibus                                           */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 2            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*/
/*                                                                           */
/* Author:                                                                   */
/*      Email   mibus@bigpond.com                                            */
/*      Webpage http://users.bigpond.com/mibus/                              */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "ency.h"

extern int optind;

void print_usage (void)
{
	printf ("findenc - Searches the Star Trek encyclopedias\nUsage: findenc [OPTION...] [search string]\n\n  --chronology\t(-c)\tSearches the chronology section\n  --episode\t(-e)\tSearches the episode guide section\n   (Default: Search the encyclopedia section)\n  --media\t(-m)\tDisplays associated media\n  --fulltext\t(-f)\tPerform a fulltext search\n  --ultraclean\t(-u)\tRemove accented characters\n");
	exit (1);
}

int main (int argc, char *argv[])
{
	int i = 0;
	char search_string[256]="";
	char *temp_fn = NULL;
	struct ency_titles *thingy = NULL;
	struct st_media *media = NULL;
	char base_path[] = "/cdrom";	/* where the media dirs etc. are */
	int use_media = 0;
	int section = ST_SECT_ENCY;
	int options = ST_OPT_MATCH_SUBSTRING | ST_OPT_RETURN_BODY | ST_OPT_NO_FMT;
	int ultraclean=0;
	static struct option long_opts[] =
	{
		{"help", 0, 0, 'h'},
		{"media", 0, 0, 'm'},
		{"episode", 0, 0, 'e'},
		{"chronology", 0, 0, 'c'},
		{"fulltext", 0, 0, 'f'},
		{"ultraclean", 0, 0, 'u'},
		{0, 0, 0, 0}};

	while ((i = getopt_long (argc, argv, "echmfu", long_opts, 0)) != EOF)
	{
		switch (i)
		{
		case 'm':
			use_media = 1;
			break;
		case 'e':
			section = ST_SECT_EPIS;
			break;
		case 'c':
			if (section == ST_SECT_EPIS)
				options = options | ST_OPT_SORTEPIS;
			else
				section = ST_SECT_CHRO;
			break;
		case 'f':
			options |= ST_OPT_FT;
			break;
		case 'u':
			ultraclean = 1;
			break;
		case 'h':
		default:
			print_usage ();
		}
	}

	/* run any ency init stuff */
	if (!st_init ())
	{
		printf ("Error opening data file.\n");
		exit (1);
	}

/* get the search string, one way or another */
	if (argc > optind)
	{
		strcpy (search_string, argv[optind]);
		for (i=optind+1;i<argc;i++)
		{
			strcat (search_string, " ");
			strcat (search_string, argv[i]);
		}
	}
	else
	{
		printf ("Enter search string: ");
		if (scanf ("%[a-zA-Z0-9.\"\'() -]", search_string) == 0) {
			printf ("No search term provided, exiting...\n");
			exit (0);
		}
	}

	/* tell ency to load the media lookup tables */
	if (use_media)
		st_load_media ();

	/* if you want to manually set the filename
	 * st_set_filename ("/dose/trek/Reference/eg_tng.dxr");
	 */

	thingy = st_find (search_string, section, options);

	/*
	 * get from a certain point in the file...
	 * thingy = st_get_title_at (0x149310);
	 */

	if ((thingy != NULL) && (thingy->title != NULL))
	{
		do
		{
			if (ultraclean)
			{
				st_ultraclean_string (thingy->name);
				st_ultraclean_string (thingy->title);
				st_ultraclean_string (thingy->text);
			}
			/* print the returned text */
			if (options & ST_OPT_FT)
				printf ("--<%s>-- [%.0f%%]\n%s\n\n%s\n\n", thingy->name, thingy->score, thingy->title, thingy->text);
			else
				printf ("--<%s>--\n%s\n\n%s\n\n", thingy->name, thingy->title, thingy->text);

			if (use_media)
				media = st_get_media (thingy->name);

			if (media)
			{
				printf ("Associated media:\n");
				for (i = 0; i < 6; i++)
					if (strlen (media->photos[i].file))
					{	/* if there is photos #i */
						temp_fn = st_format_filename (media->photos[i].file, base_path, picture);
						printf ("%s: %s\n", temp_fn, media->photos[i].caption);
						free (temp_fn);
					}
				if (strlen (media->video.file))
				{
					temp_fn = st_format_filename (media->video.file, base_path, video);
					printf ("%s: %s\n", temp_fn, media->video.caption);
					free (temp_fn);
				}
				if (strlen (media->swf.file))
				{
					temp_fn = st_format_filename (media->swf.file, base_path, swf);
					printf ("%s: %s\n", temp_fn, media->swf.caption);
					free (temp_fn);
				}
				if (strlen (media->audio.file))
				{
					temp_fn = st_format_filename (media->audio.file, base_path, audio);
					printf ("%s: %s\n", temp_fn, media->audio.caption);
					free (temp_fn);
				}
				if (strlen(media->resource))
					printf ("There is an associated resource: %s\n", media->resource);
				free (media);
				media = NULL;
			}

			st_free_entry_and_advance (&thingy);
		}
		while (thingy != NULL);

	}
	else
		printf ("No matches\n");
	st_finish ();
	return (0);
}
