#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ency.h"

/* define QUIET to print only the final total */

FILE *inp;
int tell = 0;
long old_ftell;

int guess_section (char *title, char *text, int last_section)
{
	char *episode_starts[5] =
	{
		"Original Series",
		"Next Generation",
		"Deep Space Nine",
		"Voyager episode",
		"No episodes"
	};
	int i;

	/* Episodes */
	if ((*title == '\"') || (strlen (title) == 1))
		for (i = 0; i < 5; i++)
		{
			if (!strncmp (text, episode_starts[i], strlen (episode_starts[i])))
				return 1;
			if (!strncmp (text + 1, episode_starts[i], strlen (episode_starts[i])))
				return 1;
		}
	if ((!strncmp (title, "Star Trek", 9)) && (last_section == 1))
		return 1;

	/* Chronology */
	if (*title == '\"')
		return 2;
	if (!strncmp (text, "\n\n", 2))
		return 2;
	if ((!strncmp (title, "Star Trek", 9)) && (last_section == 2))
		return 2;

	/* Encyclopedia */
	return 0;
}

int main (int argc, char *argv[])
{
	char *sections[3] =
	{"Encylopedia", "Episodes", "Chronology"};
	int counts[3] =
	{0, 0, 0};
	long returned = 0;
	int i = 0;
	char last_start = 0;
	int new_section;
	int last_section = -1;
	char *filename;
	struct ency_titles *entry;

	if (argc > 1)
	{
		filename = argv[1];
	}
	else
	{
		printf ("You must supply the main data file as the first parameter\n");
		exit (1);
	}
	inp = fopen (filename, "r b");
	if (inp == 0)
	{
		printf ("You must supply the main data file as the first parameter\n");
		exit (1);
	}
#ifndef QUIET
	printf ("Fingerprint: ");
	for (i = 0; i < 16; i++)
		printf ("%x;", getc (inp));
	printf ("\n");
#endif

	st_force_unknown_file (1);
	st_set_filename (filename);
	do
	{
		returned = st_find_start (inp);
		if (returned)
		{
			entry = st_get_title_at (ftell (inp));
			if ((!last_start) || (last_start > tolower (*entry->title)) || ((last_start == '\"') && (*entry->title != '\"')))
			{
				new_section = guess_section (entry->title, entry->text, last_section);
				if (new_section != last_section)
#ifndef QUIET
					printf ("\nNew section (%s)\n", sections[last_section = new_section]);
#else
					last_section = new_section;
#endif
			}
			last_start = tolower (*entry->title);
#ifndef QUIET
			printf ("found @ 0x%lx:  \t", ftell (inp));
			printf ("%s\n", entry->title);
#endif
			counts[last_section]++;
			st_free_entry (entry);
			getc (inp);	/* make sure it doesnt pick the same one up again */
		}
	}
	while (returned);
	printf ("Found:\n");
	for (i = 0; i < 3; i++)
		printf ("\t%s\t%d\n", sections[i], counts[i]);
	return 0;
}
