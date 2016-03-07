OUTPUT_FORMAT("binary")
ENTRY(prog_entry)
phys = 0x100000;
SECTIONS
{
  .text phys : AT(phys) {
    prog_code_section = .;
    *(.text)
    *(.rodata)
    . = ALIGN(4096);
  }
  .data : AT(phys + (data - prog_code_section))
  {
    data = .;
    *(.data)
    . = ALIGN(4096);
  }
  .bss : AT(phys + (prog_bss_section - prog_code_section))
  {
    prog_bss_section = .;
    *(.bss)
    . = ALIGN(4096);
  }
  prog_bss_end = .;
}
