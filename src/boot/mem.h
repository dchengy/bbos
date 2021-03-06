#ifndef BEANS_BOOT_MEM_H_
#define BEANS_BOOT_MEM_H_

#include <stddef.h>
#include <stdint.h>

const uint32_t SMAP_TYPE_RAM = 1;
const uint32_t SMAP_TYPE_RESERVED = 2;
const uint32_t SMAP_TYPE_ACPI_RECLAIMABLE = 3;
const uint32_t SMAP_TYPE_ACPI_NVS = 4;

struct smap_entry {
  uint64_t base;
  uint64_t size;
  uint32_t type;
  uint32_t _unused_0;
} __attribute__((packed));

void memset(void *dest, int c, size_t n) {
  unsigned char *p = (unsigned char *)dest;
  for (size_t i = 0; i < n; ++i) {
    p[i] = (unsigned char)c;
  }
}

void memcpy(void *dest, const void *src, size_t n) {
  uint8_t *d = (uint8_t *)dest;
  uint8_t *s = (uint8_t *)src;
  for (size_t i = 0; i < n; ++i) {
    d[i] = s[i];
  }
}

#endif
