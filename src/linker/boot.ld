OUTPUT_FORMAT(binary)
SECTIONS {
  . = 0x7c00;
  .boot : {
    *(.text .rodata .data .bss)
  }
}
