#include <kernel/desc.h>
#include <kernel/serial.h>
#include <kernel/printf.h>
#include <kernel/vga.h>
#include <sys/device.h>
#include <sys/kbd.h>

static char heartbeat_buf[20];
void timer_heartbeat(unsigned long t) {
  // heartbeat every 10 seconds
  if (t % 1000 == 0) {
    sprintf(heartbeat_buf, "hb %d\n", t / 1000);
    serial_write(SERIAL_PORT_COM1, heartbeat_buf);
  }
}

void kmain(void) {
  gdt_install();
  idt_install();
  irq_install();

  serial_enable(SERIAL_PORT_COM1);
  serial_write(SERIAL_PORT_COM1, "gdt, idt, irq ready\n");

  pit_install();
  pit_set_freq_hz(100);
  pit_set_timer_cb(timer_heartbeat);
  serial_write(SERIAL_PORT_COM1, "pit ready\n");

  keyboard_install();
  serial_write(SERIAL_PORT_COM1, "kbd ready\n");

  vga_init();
  serial_write(SERIAL_PORT_COM1, "vga ready\n");

  size_t kbdc = 0;
  char kbdbuf[256];

  for(;;) {
    if (kbdc == KEYBOARD_CURSOR) {
      asm("hlt");
      continue;
    }

    size_t written = 0;
    // write to end of buffer or to cursor, if wrap around
    size_t n = kbdc > KEYBOARD_CURSOR ? sizeof(KEYBOARD_BUFFER): KEYBOARD_CURSOR;
    for (size_t i = kbdc; i < n; ++i) {
      kbdbuf[i - kbdc] = scancode(KEYBOARD_BUFFER[i]);
    }
    written = n - kbdc;
    // handle wrap around
    if (n > KEYBOARD_CURSOR) {
      for (size_t i = 0; i < KEYBOARD_CURSOR; ++i) {
        kbdbuf[written + i] = scancode(KEYBOARD_BUFFER[i]);
      }
    }
    kbdbuf[written + 1] = 0;
    vga_write(kbdbuf);
    // if we get interrupt here we may skip over some keys, that's ok
    kbdc = KEYBOARD_CURSOR;
  }
}
