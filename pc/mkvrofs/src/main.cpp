/* -----------------------------------------------------------------------------
 * VROFS - Read Only File System for Embedded Systems
 * Copyright (c) 2023 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     mkvrofs.cpp
 *  brief:    Create VROFS binaries
 *  created:  2023-10-11
 *  authors:  nvitya
 *  note:
 *    append function planned later (reading back the existing file first)
*/

#include <string>
#include <vector>

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "unistd.h"
#include "string.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "vrofs.h"

using namespace std;

class TFileData
{
public:
	string      name;
	uint8_t *   fdata;
	uint32_t    size;
	uint32_t    padded_size;

	TFileData(const char * afname);
};

vector<TFileData *>  flist;

void print_usage()
{
	printf("mkvrofs <target_file> <file1> <file2>\n");
}

TFileData::TFileData(const char * afname)
{
	int          r;
	int          infd;
	struct stat  st;

	name = string(afname);

	if (stat(afname, &st) < 0)
	{
		printf("Error opening file \"%s\"\n", afname);
		exit(1);
	}

	size = st.st_size;
	padded_size = ((size + 8) & 0xFFFFFFF8);
	fdata = (uint8_t *)malloc(padded_size);
	memset(fdata, 0, padded_size);

	infd = open(afname, O_RDONLY);
	r = read(infd, fdata, size);
	if (r != int(size))
	{
		printf("Error reading file \"%s\"\n", afname);
		exit(1);
	}

	close(infd);
}

int main(int argc, char * const * argv)
{
	int i;
	uint32_t     datasize  = 0;
	uint32_t     dataoffset = 0;

  printf("MKVROFS - V1.0\n");

  if (argc < 2)
  {
  	print_usage();
  	exit(1);
  }

  const char * outfname = argv[1];

  for (i = 2; i < argc; ++i)
  {
  	TFileData * fd = new TFileData(argv[i]);
  	flist.push_back(fd);
  	printf("File added: \"%s\" - %u Bytes\n", fd->name.c_str(), fd->size);
  	datasize += fd->size;
  }

  // writing out the structure

	int outfd = open(outfname, O_TRUNC | O_CREAT | O_WRONLY, 0666);
	if (outfd < 0)
	{
		printf("Error creating \"%s\"...\n", outfname);
		exit(1);
	}

  printf("Writing \"%s\"...\n", outfname);

  // 1. write the main header

  TVrofsMainHead  mainhead;
  memset(&mainhead, 0xFF, sizeof(mainhead));  // fill the unused (reserved) fields with 0xFF
  memcpy(mainhead.vrofsid, VROFS_ID_10, 8);
  mainhead.headsize = sizeof(TVrofsMainHead);
  mainhead.indexsize = flist.size() * sizeof(TVrofsIndexRec);
  mainhead.datasize  = datasize;
  write(outfd, &mainhead, sizeof(mainhead));

  // 2. write the index

  TVrofsIndexRec irec;
  for (TFileData * fd : flist)
  {
  	memset(&irec, 0xFF, sizeof(irec)); // fill the unused (reserved) fields with 0xFF
  	memset(&irec.path[0], 0x00, sizeof(irec.path)); // except for name where pad with zeroes
  	strncpy(irec.path, fd->name.c_str(), sizeof(irec.path));
  	irec.size   = fd->size;
  	irec.offset = dataoffset;
    write(outfd, &irec, sizeof(irec));

    dataoffset += fd->padded_size;
  }

  // 3. write the data block
  for (TFileData * fd : flist)
  {
    write(outfd, fd->fdata, fd->padded_size);
  }

  printf("  %u bytes were written.\n", unsigned(lseek(outfd, 0, SEEK_END)));

  close(outfd);

  return 0;
}
