# Toshiba T100 PC
This is a project aim to emulate Toshiba T100 PC and to unveil the structure of FAT 8-bit filesystem. Specifically Toshiba T100 TDISKBASIC

## Screenshot
> [!IMPORTANT]
> **Debug** menu only appear if you manually specified `-DCMAKE_BUILD_TYPE=Debug` or `RelWithDebInfo`
> As you can see, I very suck at making UI, but good enough to use and understand what is going on
![Screenshot](./screenshot.png)

## Work-In-Progress Components
### µPD765 Floppy Disk Drive  **(Not Implemented)**
 - Port E0h: FDC TC Signal Off
 - Port E2h: FDC TC Signal On
 - Port E4h: Main Status Register (Input only)
 - Port E5h: Data Register (Read/Write)
 - Port E6h: FDC Control Port

## References
- [1]. [ToshibaT100 Technical References](https://dn721605.ca.archive.org/0/items/toshiba-t-100-tech-ref-eng/ToshibaT100_tech_ref_Eng.pdf). [Archived](./references/ToshibaT100_tech_ref_Eng.pdf)
- [2]. [NEC µPD765 Specifications](https://datasheet4u.com/pdf-down/U/P/D/UPD765-NEC.pdf). [Archived](./references/UPD765-NEC.pdf)