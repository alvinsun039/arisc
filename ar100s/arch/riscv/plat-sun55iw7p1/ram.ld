OUTPUT_ARCH("riscv")

ENTRY(_start)


MEMORY
{
	dram (rxaiw) : o = 0x40004000, l = 0x24000
}

PHDRS
{
	dram PT_LOAD;
}

SECTIONS
{
	metal_segment_stack_begin = 0x40027C00;
	metal_segment_stack_end = 0x40028000;
	PROVIDE (__etext = .);
	PROVIDE (_etext = .);
	PROVIDE (etext = .);

	.init 		:
	{
		KEEP (*(.text.metal.init.enter))
		KEEP (*(SORT_NONE(.init)))
		*crt0.o(.text .text.*)
		*e907_com.o(.text .text.*)
		*core.o(.text .text.*)
		KEEP (*(.text.libgloss.start))
	} >dram AT>dram :dram


	.text 		:
	{
		*(.text.unlikely .text.unlikely.*)
		*(.text.startup .text.startup.*)
		*main.o(.text .text.*)
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
	} >dram AT>dram :dram

	.rodata 		:
	{
		*(.rdata)
		*(.rodata .rodata.*)
		*(.gnu.linkonce.r.*)
	} >dram AT>dram :dram


	. = ALIGN(4);

	.data 		:
	{
		*(.data .data.*)
		*(.gnu.linkonce.d.*)
		. = ALIGN(8);
		PROVIDE( __global_pointer = . + 0x800 );
		*(.sdata .sdata.* .sdata2.*)
		*(.gnu.linkonce.s.*)
		. = ALIGN(8);
		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata .srodata.*)
	} >dram AT>dram :dram

		.bss		:
	{
		PROVIDE( __bss_start = .);
		*(.bss .bss.*)
		*(.sbss .sbss.*)
	} >dram AT>dram :dram
	PROVIDE(__bss_end = .);
}
