#pragma once

#include <stddef.h>
#include <stdint.h>

#define TAR_BLOCK_SIZE 512

extern uint8_t _binary_initramfs_tar_start[];
extern uint8_t _binary_initramfs_tar_end[];

void *tarfs_lookup(const char *filename, size_t *filesize, int output_flag);

struct tar_header {
  char name[100];       
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];        
  char mtime[12];
  char chksum[8];
  char typeflag;
  char linkname[100];
  char magic[6];        
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char padding[12];
};