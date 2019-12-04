%define break xchg bx, bx

KERNEL_SIZE_ADDRESS equ 0x9200
KERNEL_SECTORS_ADDR equ KERNEL_SIZE_ADDRESS + 4
KERNEL_FIRST_LBA    equ 0x0A
KERNEL_ADDRESS_ES   equ 0xfff
KERNEL_ADDRESS_BX   equ 0x10
KERNEL_READ_ADDRESS equ 0x10000
KERNEL_ADDRESS      equ 0x200000
KERNEL_SECTORS      equ 50
READ_BATCH          equ 50
SECTOR_SIZE         equ 512
HPC                 equ 2
SPT                 equ 18


[org 0x7E00]
[bits 16]
SSL:  
    jmp .readKernelSize

    ; Cylinder = LBA / (HPC * SPT)
    .computeCylinder:
        mov    bx,    ax
        xor    dx,    dx
        mov    ax,    HPC
        mov    cx,    SPT
        mul    cx
        mov    cx,    ax
        mov    ax,    bx
        div    cx
        ret

    ; Head = (LBA + SPT) % HPC
    .computeHead:
        xor    dx,    dx
        mov    cx,    SPT
        div    cx
        xor    dx,    dx
        mov    cx,    HPC
        div    cx
        mov    ax,    dx
        ret

    ; Sector = (LBA % SPT) + 1
    .computeSector:
        mov    cx,    SPT
        div    cx
        inc    dx
        mov    ax,    dx
        ret

    .LBA2CHS:
        push    ax                  ; ax = LBA
        call .computeCylinder       ; ax = Cylinder
        push    ax                  ; push Cylinder
        mov     ax,    [esp + 2]    ; ax = LBA
        call .computeHead           ; ax = Head
        push    ax                  ; push Head
        mov     ax,    [esp + 4]    ; ax = LBA
        call .computeSector         ; ax = Sector
        push    ax                  ; push Sector
        pop     cx                  ; cx = Sector
        pop     dx                  ; dx = Head
        xor     dl,     dl          ; dh = Head
        pop     bx                  ; bx = Cylinder
        mov     ax,     cx          ; ax = Sector
        mov     ch,     bl          ; ch = Cylinder
        mov     cl,     al          ; cl = Sector
        pop     ax                  ; ax = LBA
        ret
        

    ; (LBA = ax, Address = es:bx, Sectors = dx)
    .readSectors:
                                ; ax = LBA
        push    bx              ; es:bx = memory address to read
        push    es              ; es:bx = memory address to read
        push    dx              ; dx = sectors to read
        call    .LBA2CHS        ; Cylinder = ch, Head = dh, S = cl
        pop     ax              ; ax = sectors to read
        mov     ah,     02h     ; read operation => al = sectors to read
        pop     es
        pop     bx
        int     13h
        jc      .failed
        ret

    .readKernelSize:
        mov    ax,    09h                   ; starting from sector 10 - skip first 9 sectors (the MBR + SSL)
        xor    bx,    bx
        mov    es,    bx
        mov    bx,    KERNEL_SIZE_ADDRESS   ; memory from 0x7E00 - 0x9200 used by SLL;  0x9200 - 0x9FFFF is unused
        mov    dx,    1                     ; read just one sector where the fist 4 bytes are the Kernel size
        call   .readSectors

    ; Number of sectors ocupied by the kernel
    ; SECTORS = KERNEL_SIZE / SECTOR_SIZE
    .computeKernelSectors:
        mov    dx,    WORD [KERNEL_SIZE_ADDRESS]        ; dx = Most signifiant 16 bits of kernel size but in little endian
        xchg   dh,    dl                                ; dx = Most signifiant 16 bits of kernel size
        mov    ax,    WORD [KERNEL_SIZE_ADDRESS + 2]    ; ax = Less signifiant 16 bits of kernel size but in little endian
        xchg   ah,    al                                ; ax = Less signifiant 16 bits of kernel size   =>  dx:cx = kernel size
        mov    cx,    SECTOR_SIZE                       
        div    cx                                       ; ax = number of full ocupied sectors of then kernel
        test   dx,    dx
        jne    .increaseSectors                         ; if there is a sector that is not full ocupied then increase the number of sectors
        jmp    .saveKernelSectors
        .increaseSectors:
            inc ax
        .saveKernelSectors:
            mov [KERNEL_SECTORS_ADDR], ax

    .readKernelSectors:
        mov     cx,     READ_BATCH
        div     cx
        mov     cx,     ax                      ; cx = how many batches (with size = READ_BATCH) must be read until the entire kernel is loaded into memory
        test    dx,     dx
        jne     .increaseRSectors               ; if the remaining number of sectors is less then READ_BATCH increase the number of batches
        jmp     .readLoop
        .increaseRSectors:
            push dx                             ; save the remaning number of sectors on stack
            inc cx
        mov     ax,    KERNEL_ADDRESS_ES
        mov     es,    ax
        mov     ax,    KERNEL_FIRST_LBA         ; ax = LBA
        .readLoop:
            cmp     cx,     1                   ; if this is the last batch
            jne     .continueReadLoop              
            pop     dx
            test    dx,     dx
            push    dx
            jne     .readLastBatch              ; and if the last batch is incomplete read it separately
            .continueReadLoop:
            mov     bx,     KERNEL_ADDRESS_BX   ; es:bx = addreess where the kernel will be read
            mov     dx,     READ_BATCH
            push    cx
            push    ax
            push    es
            call    .readSectors                ; (LBA = ax, Address = es:bx, Sectors = dx)
            pop     ax
            add     ax,     0x1000              ; increase es with 0x1000. In this way, es:bs will always start at a multiple of 0x10000 and the 16 bit address in bx will not oferflow
            mov     es,     ax
            pop     ax
            add     ax,     READ_BATCH          ; ax = LBA + READ_BATCH
            pop     cx                          ; ecx = number of batches to be read
            loop .readLoop

        .readLastBatch:
            pop     dx                          ; dx = number of sectors that didn't complete a batch
            call    .readSectors

    jmp    .success        
  
    .failed:
        cli                ; we should reset drive and retry, but we hlt
        hlt                     
 
.success:                      
    cli                    ; starting RM to PM32 transition
    lgdt   [GDT]
    mov    eax,    cr0
    or     al,     1
    mov    cr0,    eax     ; we are in protected mode but we need to set the CS register  
    jmp    8:.bits32       ; we change the CS to 8 (index of FLAT_DESCRIPTOR_CODE32 entry)

.bits32:
[bits 32]
    mov    ax,    16       ; index of FLAT_DESCRIPTOR_DATA32 entry
    mov    ds,    ax
    mov    es,    ax      
    mov    gs,    ax      
    mov    ss,    ax      
    mov    fs,    ax 
    
    ; Copy the kernel form the space it was read in to KERNEL_ADDRESS
    cld
    xor    edx,   edx
    mov    eax,   [KERNEL_SECTORS_ADDR] ; eax = KERNEL_SECTORS
    mov    ecx,   READ_BATCH
    div    ecx
    mov    ecx,   eax                   ; ecx = number of batches that has to be moved
    mov    ebx,   KERNEL_READ_ADDRESS   ; ebx = base address to start copying the kernel
    test   edx,   edx
    jne    .increaseCSectrors           ; if there is a sector that is not full ocupied then increase the number of sectors
    jmp    .kernelCopyLoop
    .increaseCSectrors:
        push edx                        ; save the remaning number of sectors on stack
        inc  ecx
    mov     edi,    KERNEL_ADDRESS      ; edi = destination address where the kernel will be copied
    .kernelCopyLoop:
        cmp     ecx,    1               ; if this is the last batch
        jne      .continueCopyLoop
        pop     edx
        test    edx,    edx
        push    edx
        jne     .copyLastBatch          ; and if the last batch is incomplete than copy it separately
        .continueCopyLoop:
        push    ecx                     ; save the counter register
        mov     esi,    ebx             ; esi = source index to copy the kernel
        mov     ecx,    READ_BATCH * SECTOR_SIZE    ; ecx = number of bytes that has to be copied
        rep     movsb                   ; copy ecx bytes from source[esi] to destination[edi]
        add     ebx,    0x10000         ; increase source index by 0x10000 because every batch was read at an offset that is multiple of 0x10000

        pop     ecx                     ; ecx = counter for number of batches
        loop    .kernelCopyLoop

    .copyLastBatch:
        pop     ecx                     ; ecx = remaining sectors
        mov     eax,    SECTOR_SIZE 
        mul     ecx
        mov     ecx,    eax             ; ecx = number of bytes to be copied
        mov     esi,    ebx             ; esi = source index
        rep     movsb                   ; copy ecx bytes from source[esi] to destination[edi]

    
    mov    [ds:0xb8000], BYTE 'O'
    mov    [ds:0xb8002], BYTE 'K'
    
is_A20_on?:   
    ; break
    pushad
    mov edi,0x112345  ;odd megabyte address.
    mov esi,0x012345  ;even megabyte address.
    mov [esi],esi     ;making sure that both addresses contain diffrent values.
    mov [edi],edi     ;(if A20 line is cleared the two pointers would point to the address 0x012345 that would contain 0x112345 (edi)) 
    cmpsd             ;compare addresses to see if the're equivalent.
    popad
    jne A20_on        ;if not equivalent , A20 line is set.

    cli               ;if equivalent , the A20 line is cleared.
    hlt
 
A20_on:
    
    ; break
    
    mov    eax, 0x201000    ; the hardcoded ASMEntryPoint of the Kernel
    call   eax
    
    cli                     ; this should not be reached 
    hlt

;
; Data
;
GDT:
    .limit  dw  GDTTable.end - GDTTable - 1
    .base   dd  GDTTable

FLAT_DESCRIPTOR_CODE32  equ 0x00CF9A000000FFFF  ; Code: Execute/Read
FLAT_DESCRIPTOR_DATA32  equ 0x00CF92000000FFFF  ; Data: Read/Write
FLAT_DESCRIPTOR_CODE16  equ 0x00009B000000FFFF  ; Code: Execute/Read, accessed
FLAT_DESCRIPTOR_DATA16  equ 0x000093000000FFFF  ; Data: Read/Write, accessed
FLAT_DESCRIPTOR_CODE64  equ 0x00AF9A000000FFFF  ; Code: Execute/Read
FLAT_DESCRIPTOR_DATA64  equ 0x00AF92000000FFFF  ; Data: Read/Write

GDTTable:
    .null     dq 0                         ;  0
    .code32   dq FLAT_DESCRIPTOR_CODE32    ;  8
    .data32   dq FLAT_DESCRIPTOR_DATA32    ; 16
    .code16   dq FLAT_DESCRIPTOR_CODE16    ; 24
    .data16   dq FLAT_DESCRIPTOR_DATA16    ; 32
    .data64   dq FLAT_DESCRIPTOR_DATA64    ; 40
    .code64   dq FLAT_DESCRIPTOR_CODE64    ; 48
    .end:
