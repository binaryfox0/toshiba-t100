# Toshiba T100 PC
## Screenshot
> [!IMPORTANT]
> **Debug** menu only appear if you manually specified `-DCMAKE_BUILD_TYPE=Debug` or `RelWithDebInfo`
> As you can see, I very suck at making UI, but good enough to use and understand what is going on

![Screenshot](./screenshot.png)

## Implementation Status
> [!IMPORTANT]
> Components aren't included in the list below won't be implemented for now

|Components|Device name|Official docs|Code behaviour|Status|
|-|-|-|-|-|
|Floppy Disk Controller|NEC µPD765|[Docs](./references/UPD765-NEC.pdf)|[Behaviour](./research/uPD765.md)|WIP
|Keyboard|Proprietary?|[Docs](./references/ToshibaT100_tech_ref_Eng.pdf)|Unknown|❌

## Known issues
- There is a memory leak, but that's not my fault at all, maybe. By using `valgrind`, it seems to leak from SDL2 (i use arch btw)
<details>

```
==16780== 
==16780== HEAP SUMMARY:
==16780==     in use at exit: 285,537 bytes in 2,876 blocks
==16780==   total heap usage: 157,991 allocs, 155,115 frees, 48,805,720 bytes allocated
==16780== 
==16780== 3,056 (2,688 direct, 368 indirect) bytes in 1 blocks are definitely lost in loss record 2,741 of 2,749
==16780==    at 0x4A2AC13: calloc (vg_replace_malloc.c:1675)
==16780==    by 0xACF30EF: ???
==16780==    by 0xACF382F: ???
==16780==    by 0xAC8325D: ???
==16780==    by 0x9561DFA: UnknownInlinedFun (SDL_egl.c:554)
==16780==    by 0x9561DFA: SDL_EGL_LoadLibrary (SDL_egl.c:506)
==16780==    by 0x95DA278: Wayland_GLES_LoadLibrary (SDL_waylandopengles.c:42)
==16780==    by 0x9581A3D: SDL_GL_LoadLibrary_REAL (SDL_video.c:4417)
==16780==    by 0x9586547: SDL_RecreateWindow (SDL_video.c:2668)
==16780==    by 0x94E8B9F: GL_CreateRenderer.lto_priv.0 (SDL_render_gl.c:1645)
==16780==    by 0x94D176D: SDL_CreateRendererWithProperties_REAL (SDL_render.c:1037)
==16780==    by 0x94D1956: SDL_CreateRenderer_REAL (SDL_render.c:1166)
==16780==    by 0x4A866D8: ??? (in /usr/lib/libSDL2-2.0.so.0.3200.56)
==16780== 
==16780== LEAK SUMMARY:
==16780==    definitely lost: 2,688 bytes in 1 blocks
==16780==    indirectly lost: 368 bytes in 4 blocks
==16780==      possibly lost: 0 bytes in 0 blocks
==16780==    still reachable: 280,633 bytes in 2,850 blocks
==16780==         suppressed: 0 bytes in 0 blocks
==16780== Reachable blocks (those to which a pointer was found) are not shown.
==16780== To see them, rerun with: --leak-check=full --show-leak-kinds=all
==16780== 
==16780== ERROR SUMMARY: 3 errors from 3 contexts (suppressed: 0 from 0)
==16780== 
==16780== 1 errors in context 1 of 3:
==16780== posix_memalign() invalid size value: 0
==16780==    at 0x4A2BA58: posix_memalign (vg_replace_malloc.c:2226)
==16780==    by 0xC386D53: ???
==16780==    by 0xC38BFCB: ???
==16780==    by 0xC39CF38: ???
==16780==    by 0xC39D314: ???
==16780==    by 0xC39836E: ???
==16780==    by 0xAE354B4: ???
==16780==    by 0xAE3559A: ???
==16780==    by 0xAEAD3D5: ???
==16780==    by 0xAE33527: ???
==16780==    by 0xAE486EC: ???
==16780==    by 0xACAA8D7: ???
==16780== 
==16780== 
==16780== 1 errors in context 2 of 3:
==16780== realloc() with size 0
==16780==    at 0x4A2AE4F: realloc (vg_replace_malloc.c:1801)
==16780==    by 0xC386D22: ???
==16780==    by 0xC38BFCB: ???
==16780==    by 0xC39CF38: ???
==16780==    by 0xC39D314: ???
==16780==    by 0xC39836E: ???
==16780==    by 0xAE354B4: ???
==16780==    by 0xAE3559A: ???
==16780==    by 0xAEAD3D5: ???
==16780==    by 0xAE33527: ???
==16780==    by 0xAE486EC: ???
==16780==    by 0xACAA8D7: ???
==16780==  Address 0x944cfe0 is 0 bytes after a block of size 0 alloc'd
==16780==    at 0x4A237A8: malloc (vg_replace_malloc.c:446)
==16780==    by 0xC386D0F: ???
==16780==    by 0xC38BFCB: ???
==16780==    by 0xC39CF38: ???
==16780==    by 0xC39D314: ???
==16780==    by 0xC39836E: ???
==16780==    by 0xAE354B4: ???
==16780==    by 0xAE3559A: ???
==16780==    by 0xAEAD3D5: ???
==16780==    by 0xAE33527: ???
==16780==    by 0xAE486EC: ???
==16780==    by 0xACAA8D7: ???
==16780== 
==16780== ERROR SUMMARY: 3 errors from 3 contexts (suppressed: 0 from 0)
```
</details>

## References
- [1]. [ToshibaT100 Technical References](https://dn721605.ca.archive.org/0/items/toshiba-t-100-tech-ref-eng/ToshibaT100_tech_ref_Eng.pdf). [Archived](./references/ToshibaT100_tech_ref_Eng.pdf)
- [2]. [NEC µPD765 Specifications](https://datasheet4u.com/pdf-down/U/P/D/UPD765-NEC.pdf). [Archived](./references/UPD765-NEC.pdf)