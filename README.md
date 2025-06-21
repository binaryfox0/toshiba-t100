# Toshiba T100 PC
This is a project aim to emulate Toshiba T100 PC and to unveil the structure of FAT 8-bit filesystem. Specifically Toshiba T100 TDISKBASIC

## Components
### µPD765 Floppy Disk Drive  **(Not Implemented)**
 - Port E0h: FDC TC Signal Off
 - Port E2h: FDC TC Signal On
 - Port E4h: Status Register (Input only)
 - Port E5h: Data Register (Read/Write)
 - Port E6h: FDC Control Port
#### Details
- With load address 0xD000 with file offset 0x1000. `D758h - D763h` seems to be where the loop trying to read status register.

```nasm
ld a, 0x01         ; A = 0x01
call 0xD470        ; Call subroutine at D470
    push de            ; Preserve DE
    ld e, a            ; E = A
    ld d, 0x00         ; D = 0 → DE = 0x00XX (8-bit offset)
    add hl, de         ; HL += A
    pop de             ; Restore DE
    ret
in a, (0xE4)       ; Read from I/O port 0xE4 into A
inc a              ; A = A + 1
ret z              ; Return if result == 0 (A was 0xFF → overflow to 0x00)
ld a, h            ; A = H
or l               ; A = A | L
jp nz, 0xd758      ; Jump if A != 0
ret                ; Final return
```

## References
- [1]. [ToshibaT100 Technical References](https://dn721605.ca.archive.org/0/items/toshiba-t-100-tech-ref-eng/ToshibaT100_tech_ref_Eng.pdf). [Archived](./references/ToshibaT100_tech_ref_Eng.pdf)
- [2]. [NEC µPD765 Specifications](https://datasheet4u.com/pdf-down/U/P/D/UPD765-NEC.pdf). [Archived](./references/UPD765-NEC.pdf)