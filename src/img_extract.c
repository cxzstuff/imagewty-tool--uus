/**
 * @file img_extract.c
 * @brief Implements extraction of all files from an IMAGEWTY image.
 *
 * This module extracts files from an IMAGEWTY image into a dump folder,
 * writes an image.cfg with metadata, and verifies integrity using
 * V*.fex checksums.
 */
#ifdef _WIN32
#include <direct.h>
#endif
#include <errno.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "checksum.h"
#include "config_file.h"
#include "img_header.h"

/**
 * @brief Extract all files from an IMAGEWTY image into a dump folder.
 *
 * After extraction, a config file (image.cfg) is generated and V*.fex
 * checksums are verified (without updating them).
 *
 * @param img_filename Path to the IMAGEWTY image file.
 * @return 0 on success, non-zero on error.
 */
int extract_image(const char* img_filename)
{
    if (!img_filename)
    {
        fprintf(stderr, "extract_image: img_filename is NULL\n");
        return 1;
    }

    FILE* f = fopen(img_filename, "rb");
    if (!f)
    {
        perror("Error opening image file");
        return 1;
    }

    /* Read main image header */
    ImageWTYHeader hdr;
    if (read_image_header(f, &hdr) != 0)
    {
        fprintf(stderr, "Error: Failed to read image header from '%s'\n", img_filename);
        fclose(f);
        return 1;
    }

    if (strncmp(hdr.magic, IMAGEWTY_MAGIC, 8) != 0)
    {
        fprintf(stderr, "Error: '%s' is not a valid IMAGEWTY image.\n", img_filename);
        fclose(f);
        return 1;
    }

    /* Read all file headers */
    ImageWTYFileHeader* files = read_all_file_headers(f, hdr.num_files, hdr.file_header_length);
    if (!files)
    {
        fprintf(stderr, "Error reading file headers from '%s'\n", img_filename);
        fclose(f);
        return 1;
    }

    /* Create dump directory: <image>.dump
     * Copy the filename to a local buffer before calling basename(),
     * since POSIX basename() may modify the input string. */
    char name_buf[1024];
    snprintf(name_buf, sizeof(name_buf), "%s", img_filename);

    char dump_dir[1024];
    snprintf(dump_dir, sizeof(dump_dir), "%s.dump", basename(name_buf));
  //  if (mkdir(dump_dir, 0755) && errno != EEXIST)
     #ifdef _WIN32
    if (mkdir(dump_dir) && errno != EEXIST)
    #else
    if (mkdir(dump_dir, 0755) && errno != EEXIST)
    #endif
    {
        perror("Error creating dump directory");
        free(files);
        fclose(f);
        return 1;
    }

    /* Write image.cfg inside the dump directory */
    char cfg_path[1024];
    size_t dump_len = strlen(dump_dir);
    const char* suffix = "/image.cfg";

    if (dump_len + strlen(suffix) >= sizeof(cfg_path))
    {
        fprintf(stderr, "Error: dump directory path too long for image.cfg\n");
    }
    else
    {
        snprintf(cfg_path, sizeof(cfg_path), "%s%s", dump_dir, suffix);

        if (write_image_config(cfg_path, &hdr, files, hdr.num_files) == 0)
        {
            printf("image.cfg successfully written as '%s'\n", cfg_path);
        }
        else
        {
            fprintf(stderr, "Failed to write image.cfg\n");
        }
    }

    /* Extract each file from the image */
    for (uint32_t i = 0; i < hdr.num_files; i++)
    {
        ImageWTYFileHeader* fh = &files[i];
        char filepath[1024];

        if (snprintf(filepath, sizeof(filepath), "%s/%s", dump_dir, fh->filename) >=
            (int)sizeof(filepath))
        {
            fprintf(stderr, "File path too long, skipping '%s'\n", fh->filename);
            continue;
        }

        FILE* of = fopen(filepath, "wb");
        if (!of)
        {
            perror("Error creating output file");
            continue;
        }

        if (fseek(f, fh->offset, SEEK_SET) != 0)
        {
            perror("Error seeking file data");
            fclose(of);
            continue;
        }

        uint8_t* data = malloc(fh->original_length);
        if (!data)
        {
            perror("Memory allocation failed");
            fclose(of);
            continue;
        }

        if (fread(data, 1, fh->original_length, f) != fh->original_length)
        {
            fprintf(stderr, "Error reading file data for '%s'\n", fh->filename);
            free(data);
            fclose(of);
            continue;
        }

        printf("Extracting: %s (%u bytes)\n", filepath, fh->original_length);
        if (fwrite(data, 1, fh->original_length, of) != fh->original_length)
        {
            fprintf(stderr, "Error writing file '%s'\n", filepath);
        }
        free(data);
        fclose(of);
    }

    free(files);
    fclose(f);

    /* Verify integrity using V*.fex checksums (without updating them) */
    printf("\nVerifying extracted files using V*.fex checksums...\n");
    verify_vfiles_checksums(dump_dir);

    return 0;
}
