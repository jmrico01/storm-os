SECTIONS
{
    . = 0x100000;
    textStartLD = . ;
    .text : { *(.text) }
    textEndLD = . ;

    . = 0x110000;
    dataStartLD = . ;
    .data : { *(.data) }
    .bss  : { *(.bss)  }
    dataEndLD = . ;
}