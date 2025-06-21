# Toshiba T100 PC
This is a project aim to emulate Toshiba T100 PC and to unveil the structure of FAT 8-bit filesystem. Specifically Toshiba T100 TDISKBASIC

> [!IMPORTANT]
> All things below will assume that you load the IPL at address 0xD000

## Components
### µPD765 Floppy Disk Drive  **(Not Implemented)**
 - Port E0h: FDC TC Signal Off
 - Port E2h: FDC TC Signal On
 - Port E4h: Main Status Register (Input only)
 - Port E5h: Data Register (Read/Write)
 - Port E6h: FDC Control Port
#### Details
- First read on port E4h is at address D1FBh and was called by instruction `D1C6: jr z,36`
```nasm
D1FB: in a, (0xE4)  ; Read Main Status Register
D1FD: or a          ; Check if read byte is zero
D1FE: jr z, 3       ; Jump relatively +3 bytes (D203)
D200: inc a         ; A++
D201: jr nz, 63     ; Jump relatively +63 bytes if a is not zero.
                    ; means D1FB instruction must make A be FF. At D1FE it will not jump,
                    ; Instruction after that will make A overflow to 0
D203: out (0xE6), a ; Respond FDC Control Port with value it read from MSR
D205: dec a         ; A--

D242: 
```

- `D758h - D763h` seems to be where the loop trying to read status register if you reply with 00h when it read from port E4h

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