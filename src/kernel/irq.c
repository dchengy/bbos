#include <stddef.h>
#include <stdint.h>

#include <kernel/desc.h>
#include <kernel/serial.h>
#include <sys/io.h>

static const uint8_t PIC_ACK = 0x20;
static const uint8_t ICW1_ICW4 = 0x01;
static const uint8_t ICW1_INIT = 0x10;
static const uint8_t PIC1_OFFSET = 0x20;
static const uint8_t PIC2_OFFSET = 0x28;
static const uint8_t ICW4_8086 = 0x01;
static const uint8_t KCODE_SELECTOR = 0x08;
static const uint8_t INTERRUPT32_PRESENT = 0x8e;
static const uint8_t TRAP32_PRESENT = 0x8f;

extern void irq_handler_start32();
extern void irq_handler_start33();
extern void irq_handler_start34();
extern void irq_handler_start35();
extern void irq_handler_start36();
extern void irq_handler_start37();
extern void irq_handler_start38();
extern void irq_handler_start39();
extern void irq_handler_start40();
extern void irq_handler_start41();
extern void irq_handler_start42();
extern void irq_handler_start43();
extern void irq_handler_start44();
extern void irq_handler_start45();
extern void irq_handler_start46();
extern void irq_handler_start47();

static __attribute__((used)) void (*irqs[16])(void) = {
    irq_handler_start32, irq_handler_start33, irq_handler_start34,
    irq_handler_start35, irq_handler_start36, irq_handler_start37,
    irq_handler_start38, irq_handler_start39, irq_handler_start40,
    irq_handler_start41, irq_handler_start42, irq_handler_start43,
    irq_handler_start44, irq_handler_start45, irq_handler_start46,
    irq_handler_start47,
};
static __attribute__((used)) int (*irq_handlers[16])(struct irq_state *s) = {};

static inline void disable_int() { asm volatile("cli"); }

static inline void enable_int() { asm volatile("sti"); }

static inline uint8_t pic_command(enum pic_port p) { return p; }

static inline uint8_t pic_data(enum pic_port p) { return p + 1; }

static void pic_remap() {
  uint8_t old_p1 = inb(pic_data(PIC1));
  uint8_t old_p2 = inb(pic_data(PIC2));

  outb(pic_command(PIC1), ICW1_ICW4 | ICW1_INIT);
  iowait();
  outb(pic_command(PIC2), ICW1_ICW4 | ICW1_INIT);
  iowait();

  outb(pic_data(PIC1), PIC1_OFFSET);
  iowait();
  outb(pic_data(PIC2), PIC2_OFFSET);
  iowait();

  // set up parent (PIC1) and child (PIC2)
  outb(pic_data(PIC1), 0x04);
  iowait();
  outb(pic_data(PIC2), 0x02);
  iowait();

  // request 8086 mode
  outb(pic_data(PIC1), ICW4_8086);
  iowait();
  outb(pic_data(PIC2), ICW4_8086);
  iowait();

  outb(pic_data(PIC1), old_p1);
  iowait();
  outb(pic_data(PIC2), old_p2);
  iowait();
}

void c_irq_handler(struct irq_state *s) {
  disable_int();
  if (s->interrupt >= PIC1_OFFSET &&
      s->interrupt <= (unsigned int)PIC2_OFFSET + 8) {
    size_t i = s->interrupt - PIC1_OFFSET;
    if (!irq_handlers[i] || !irq_handlers[i](s)) {
      // no handler or our handler failed
      pic_ack(i);
    }
  }
  enable_int();
}

void pic_ack(size_t irq) {
  outb(pic_command(PIC1), PIC_ACK);
  if (irq >= 8) {
    outb(pic_command(PIC2), PIC_ACK);
  }
}

void irq_install() {
  pic_remap();
  size_t n = sizeof(irqs) / sizeof(irqs[0]);
  for (size_t i = 0; i < n; ++i) {
    idt_set_gate(PIC1_OFFSET + i,   // gate
                 (uint32_t)irqs[i], // offset
                 KCODE_SELECTOR, INTERRUPT32_PRESENT);
  }
}

void irq_install_isr(size_t irq, int (*handler)(struct irq_state *)) {
  disable_int();
  irq_handlers[irq] = handler;
  enable_int();
}

void irq_install_trap(size_t irq, int (*handler)(struct irq_state *)) {
  idt_set_gate(irq, (uint32_t)handler, KCODE_SELECTOR, TRAP32_PRESENT);
}
