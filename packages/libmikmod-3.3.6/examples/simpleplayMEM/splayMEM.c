/* splayMEM.c
 * An example on how to use libmikmod to play
 * a module, but to load it with a custom MREADER.
 *
 * (C) 2004, Raphael Assenat (raph@raphnet.net)
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRENTY; without event the implied warrenty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <mikmod.h>
#if !defined(_WIN32)
#include <unistd.h>  /* for usleep() */
#define MikMod_Sleep(ns) usleep(ns)
#else
#define MikMod_Sleep(ns) Sleep(ns / 1000)
#endif

#include "mmloader.h"

int main(int argc, char **argv)
{
	MODULE *module;
	unsigned char *file_data;
	long sysFileGetLength;
	FILE *fptr;
	MREADER *mem_reader;

	if (argc<2) {
		fprintf(stderr, "Usage: ./splayMEM file\n");
		return 1;
	}

	/* initialize MikMod threads */
	MikMod_InitThreads ();

	/* register all the drivers */
	MikMod_RegisterAllDrivers();

	/* register all the module loaders */
	MikMod_RegisterAllLoaders();

	/* init the library */
	md_mode |= DMODE_SOFT_MUSIC | DMODE_NOISEREDUCTION;
	md_mode |= DMODE_HQMIXER;
	/*md_mode |= DMODE_SIMDMIXER;*/

	if (MikMod_Init("")) {
		fprintf(stderr, "Could not initialize sound, reason: %s\n",
				MikMod_strerror(MikMod_errno));
		return 2;
	}

	/* open the file */
	fptr = fopen(argv[1], "rb");
	if (fptr==NULL) {
		perror("fopen");
		MikMod_Exit();
		return 1;
	}

	/* calculate the file size */
	fseek(fptr, 0, SEEK_END);
	sysFileGetLength = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);

	/* allocate a buffer and load the file into it */
	file_data = MikMod_malloc(sysFileGetLength);
	if (file_data == NULL) {
		perror("MikMod_malloc");
		fclose(fptr);
		MikMod_Exit();
		return 1;
	}
	if (fread(file_data, sysFileGetLength, 1, fptr) != 1)
	{
		perror("fread");
		fclose(fptr);
		MikMod_free(file_data);
		MikMod_Exit();
		return 1;
	}
	fclose(fptr);

	/* Create the memory reader */
	mem_reader = my_new_mem_reader(file_data, sysFileGetLength);
	if (mem_reader == NULL) {
		fprintf(stderr, "failed to create mem reader\n");
		MikMod_free(file_data);
		MikMod_Exit();
		return 1;
	}

	/* load module */
	module = Player_LoadGeneric(mem_reader, 64, 0);
	if (module) {
		/* start module */
		printf("Playing %s\n", module->songname);
		Player_Start(module);

		while (Player_Active()) {
			MikMod_Sleep(10000);
			MikMod_Update();
		}

		Player_Stop();
		Player_Free(module);
	} else
		fprintf(stderr, "Could not load module, reason: %s\n",
				MikMod_strerror(MikMod_errno));

	my_delete_mem_reader(mem_reader);
	mem_reader = NULL;
	MikMod_free(file_data);
	MikMod_Exit();

	return 0;
}

