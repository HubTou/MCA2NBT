/* @(#) mca2nbt:mca2nbt.c v1.00 (2014-05-05) / Hubert Tournier */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "minecraft.h"

static const char VersionId[] = "@(#) mca2nbt v1.00 (2014-05-05) / Hubert Tournier";

int main(int argc, char *argv[])
{
	int	region;

	if (argc < 2)
	{
		fprintf(stderr, "%s\n", &VersionId[5]);
		fprintf(stderr, "Usage: %s file.mca ...\n", argv[0]);
		exit(1);
	}

	for (region = 1; region < argc; region++)
	{
		FILE 			*input_file,
					*header_output_file;
		char			output_directory_name[1024],
					header_output_file_path[1024],
					*p_file_extension;
		RegionFileHeader	header;
		int			i;

		/* make a directory to receive the chunk files for this region */
		/***************************************************************/
		strcpy(output_directory_name, argv[region]);
		if ((p_file_extension = strstr(output_directory_name, ".mca")) == NULL)
		{
			fprintf(stderr, "FATAL ERROR: file name \"%s\" has no .mca extension\n", argv[region]);
			exit(2);
		}
		else
		{
			*p_file_extension = 0;
		}
		if (mkdir(output_directory_name, "0755") != 0)
		{
			if (errno != EEXIST)
			{
				fprintf(stderr, "FATAL ERROR: unable to create the destination directory \"%s\"\nReason: %s\n", argv[region], sys_errlist[errno]);
				exit(3);
			}
		}
	

		/* read the region file header */
		/*******************************/
		if ((input_file = fopen(argv[region], "r")) == NULL)
		{
			fprintf(stderr, "FATAL ERROR: unable to open \"%s\"\n", argv[region]);
			exit(4);
		}
		if (fread(&header, sizeof(RegionFileHeader), 1, input_file) != 1)
		{
			fprintf(stderr, "FATAL ERROR: unable to read the region file header for \"%s\"\n", argv[region]);
			exit(5);
		}
	
		/* write the region file header to a file                                            */
		/* (in case someone would want to reconstruct the MCA file from the directory files) */
		/*************************************************************************************/
		sprintf(header_output_file_path, "%s/header.bin", output_directory_name);
		if ((header_output_file = fopen(header_output_file_path, "w")) == NULL)
		{
			fprintf(stderr, "FATAL ERROR: unable to open file \"%s\" for writing header\n", header_output_file_path);
			exit(6);
		}
		if (fwrite(&header, sizeof(RegionFileHeader), 1, header_output_file) != 1)
		{
			fprintf(stderr, "FATAL ERROR: unable to write header to file \"%s\"\n", header_output_file_path);
			exit(7);
		}
		fclose(header_output_file);


		/* for each chunk entry... */
		/***************************/
		for (i = 0; i < CHUNKS_PER_REGION; i++)
		{
			unsigned int	offset,
					length;
			ChunkDataHeader	header_chunk;
	
			offset = (65536 * header.locations[i].offset[0]) + (256 * header.locations[i].offset[1]) + header.locations[i].offset[2];
	
			if (offset != 0)
			{
				char	*chunk;
				FILE	*deflated_output_file,
					*uncompressed_output_file;
				char	deflated_output_file_path[1024],
					uncompressed_output_file_path[1024];
	
				/* read the chunk header */
				/*************************/
				if (fseek(input_file, offset * 4096, SEEK_SET) == -1)
				{
					fprintf(stderr, "FATAL ERROR: unable to seek to chunk #d in \"%s\"\n", i, argv[region]);
					exit(8);
				}
				if (fread(&header_chunk, sizeof(ChunkDataHeader), 1, input_file) != 1)
				{
					fprintf(stderr, "FATAL ERROR: unable to read chunk #%d header in \"%s\"\n", i, argv[region]);
					exit(9);
				}
				length = (16777216 * header_chunk.length[0]) + (65536 * header_chunk.length[1]) + (256 * header_chunk.length[2]) + header_chunk.length[3];
	
				/* verify structure */
				/********************/
				if (length > 4096 * header.locations[i].sector_count)
				{
					fprintf(stderr, "FATAL ERROR: chunk #%d is bigger than expected (%u bytes vs %u) in \"%s\"\n", i, length, 4096 * header.locations[i].sector_count, argv[region]);
					exit(10);
				}
				if (header_chunk.compression_type != COMPRESSION_TYPE_GZIP && header_chunk.compression_type != COMPRESSION_TYPE_ZLIB)
				{
					fprintf(stderr, "FATAL ERROR: chunk #%d has unknown compression type (%u) in \"%s\"\n", i, header_chunk.compression_type, argv[region]);
					exit(11);
				}
				else if (header_chunk.compression_type == COMPRESSION_TYPE_GZIP)
				{
					fprintf(stderr, "FATAL ERROR: chunk #%d is compressed with gzip, that this program doesn't support, in \"%s\"\n", i, argv[region]);
					exit(12);
				}
	
				/* read the rest of the chunk */
				/******************************/
				if ((chunk = malloc(length - 1)) == NULL)
				{
					fprintf(stderr, "FATAL ERROR: unable to alloc %u bytes of memory to read chunk #%d in \"%s\"\n", length, i, argv[region]);
					exit(13);
				}
				if (fread(chunk, length - 1, 1, input_file) != 1)
				{
					fprintf(stderr, "FATAL ERROR: unable to read chunk #%d data in \"%s\"\n", i, argv[region]);
					free(chunk);
					exit(14);
				}
	
				/* write the chunk as an individual deflated NBT file */
				/******************************************************/
				sprintf(deflated_output_file_path, "%s/%d.nbt.zlib", output_directory_name, i);
				if ((deflated_output_file = fopen(deflated_output_file_path, "w")) == NULL)
				{
					fprintf(stderr, "FATAL ERROR: unable to open file \"%s\" for writing chunk #%d\n", deflated_output_file_path, i);
					free(chunk);
					exit(15);
				}
				if (fwrite(chunk, length - 1, 1, deflated_output_file) != 1)
				{
					fprintf(stderr, "FATAL ERROR: unable to write chunk #%d data to file \"%s\"\n", i, deflated_output_file_path);
					free(chunk);
					exit(16);
				}
				fclose(deflated_output_file);
				free(chunk);
	
				/* write the chunk as an individual uncompressed NBT file and remove the previous one */
				/**************************************************************************************/
				/* (i could have used fmemopen instead of stdin, but i'm lazy tonight!) */
				freopen(deflated_output_file_path, "r", stdin);
				sprintf(uncompressed_output_file_path, "%s/%d.nbt", output_directory_name, i);
				freopen(uncompressed_output_file_path, "w", stdout);
				zpipe();
				unlink(deflated_output_file_path);
			}
		}
		fclose(input_file);
	}
}

