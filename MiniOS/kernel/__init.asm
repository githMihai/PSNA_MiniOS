;;-----------------_DEFINITIONS ONLY_-----------------------
;; IMPORT FUNCTIONS FROM C
%macro IMPORTFROMC 1-*
%rep  %0
    %ifidn __OUTPUT_FORMAT__, win32 ; win32 builds from Visual C decorate C names using _ 
    extern _%1
    %1 equ _%1
    %else
    extern %1
    %endif
%rotate 1 
%endrep
%endmacro

;; EXPORT TO C FUNCTIONS
%macro EXPORT2C 1-*
%rep  %0
    %ifidn __OUTPUT_FORMAT__, win32 ; win32 builds from Visual C decorate C names using _ 
    global _%1
    _%1 equ %1
    %else
    global %1
    %endif
%rotate 1 
%endrep
%endmacro

%define break xchg bx, bx

IMPORTFROMC KernelMain, IRQ0_handler, IRQ1_handler, IRQ2_handler, IRQ3_handler, IRQ4_handler, IRQ5_handler, IRQ6_handler, IRQ7_handler, IRQ8_handler, IRQ9_handler, ERQ0_handler, ERQ5_handler, ERQ8_handler, ERQ14_handler

TOP_OF_STACK                equ 0x200000
KERNEL_BASE_PHYSICAL        equ 0x200000
PML4E_ADDR                  equ 0x401000
PDPTE_ADDR                  equ 0x402000
PDE_ADDR                    equ 0x403000
PT_0_ADDR                   equ 0x404000
PT_1_ADDR                   equ 0x405000
PT_2_ADDR                   equ 0x406000
PT_3_ADDR                   equ 0x409000
PAGE_SIZE                   equ 0x1000  ;4KB

READ_WRITE_PRESENT          equ 0x3
PAGE_ADDRESS_EXTENSION      equ 0b100000
PT_SIZE                     equ 512
PTE_SIZE                    equ 8
;;-----------------^DEFINITIONS ONLY^-----------------------

segment .text
[BITS 32]
ASMEntryPoint:
    cli
    MOV     DWORD [0x000B8000], 'O1S1'
%ifidn __OUTPUT_FORMAT__, win32
    MOV     DWORD [0x000B8004], '3121'                  ; 32 bit build marker
%else
    MOV     DWORD [0x000B8004], '6141'                  ; 64 bit build marker
%endif

call __enableSSE

    MOV     ESP, TOP_OF_STACK                           ; just below the kernel
    
    ; break

    ;TODO!!! define page tables; see https://wiki.osdev.org ,Intel's manual, http://www.brokenthorn.com/Resources/
        ; Clear memory
    mov edi, PML4E_ADDR    ; Set the destination index to 0x1000.
    mov CR3, edi       ; Set control register 3 to the destination index.
    xor eax, eax       ; Nullify the A-register.
    mov ecx, 4096      ; Set the C-register to 4096.
    rep stosd          ; Clear the memory.
    mov edi, CR3       ; Set the destination index to control register 3.

    ; PAGE MAP LEVEL 4
    PML4:
        mov QWORD eax, PDPTE_ADDR   ; Put Page Directory Pointer address intro eax.
        or eax, READ_WRITE_PRESENT  ; Set the bits for read/write and present.
        mov [PML4E_ADDR], eax       ; Make the entry 0 intro the Page Map Level 4 Table to point to a Page Directory Pointer Table.

    ; Page Directory Pointer
    PDP:
        mov QWORD eax, PDE_ADDR     ; Put Page Directory address into eax.
        or eax, READ_WRITE_PRESENT  ; Set the bits for read/write and present.
        mov [PDPTE_ADDR], eax       ; Make the entry 0 into the Page Directory Pointer Table to point to Page Directory.

    ; Page Directory
    PD:
        mov QWORD eax, PT_0_ADDR    ; Put the first Page Table address into eax.
        or eax, READ_WRITE_PRESENT  ; Set the bits for read/write and present.
        mov [PDE_ADDR], eax         ; Make the entry 0 into the Page Directory table to point to first Page Table.

        mov QWORD eax, PT_1_ADDR        ; Put the second Page Table address into eax.
        or eax, READ_WRITE_PRESENT      ; Set the bits for read/write and present.
        mov [PDE_ADDR+PTE_SIZE], eax    ; Make the entry 1 into the Page Directory table to point to second Page Table.

        mov QWORD eax, PT_2_ADDR        ; Put the third Page Table address into eax.
        or eax, READ_WRITE_PRESENT      ; Set the bits for read/write and present.
        mov [PDE_ADDR+2*PTE_SIZE], eax  ; Make the entry 0 into the Page Directory table to point to third Page Table.

    ; First Page Table
    PT_0:
        xor eax, eax          ; Set eax to 0 because we want to begin identity mapping form the fist address in memory wich will probably be a location in stack.
        mov edi, PT_0_ADDR  ; Set the destination index register to point to fist entry in PT_1.
        mov ecx, PT_SIZE    ; Set the count register to 512 (number of entries in a bage table).
        .loop_0:                        
            or eax, READ_WRITE_PRESENT  ; Set all pages form the Page Table to be readable/writable and present.
            mov DWORD [edi], eax        ; Put the physical address of the page into the coresponding entry in Page Table.
            add eax, PAGE_SIZE          ; Compute the physical address of the next page.
            add edi, PTE_SIZE           ; Increase the destiantion index into the page table.
            loop .loop_0

    ; Second Page Table
    PT_1:
        mov edi, PT_1_ADDR  ; Set the destination index register to point to fist entry in PT_1.
        mov ecx, PT_SIZE    ; Set the count register to 512 (number of entries in a bage table).
        .loop_1:                        
            or eax, READ_WRITE_PRESENT  ; Set all pages form the Page Table to be readable/writable and present.
            mov DWORD [edi], eax        ; Put the physical address of the page into the coresponding entry in Page Table.
            add eax, PAGE_SIZE          ; Compute the physical address of the next page.
            add edi, PTE_SIZE           ; Increase the destiantion index into the page table.
            loop .loop_1

    ; Third Page Table
    PT_2:
        mov edi, PT_2_ADDR  ; Set the destination index register to point to fist entry in PT_2.
        mov ecx, PT_SIZE    ; Set the count register to 512 (number of entries in a bage table).
        .loop_2:
            or eax, READ_WRITE_PRESENT  ; Set all pages form the Page Table to be readable/writable and present.
            mov DWORD [edi], eax        ; Put the physical address of the page into the coresponding entry in Page Table.
            add eax, PAGE_SIZE          ; Compute the physical address of the next page.
            add edi, PTE_SIZE           ; Increase the destiantion index into the page table.
            loop .loop_2
    ; Forth Page Table
    PT_3:
        mov edi, PT_3_ADDR  ; Set the destination index register to point to fist entry in PT_2.
        mov ecx, PT_SIZE    ; Set the count register to 512 (number of entries in a bage table).
        .loop_3:
            or eax, READ_WRITE_PRESENT  ; Set all pages form the Page Table to be readable/writable and present.
            mov DWORD [edi], eax        ; Put the physical address of the page into the coresponding entry in Page Table.
            add eax, PAGE_SIZE          ; Compute the physical address of the next page.
            add edi, PTE_SIZE           ; Increase the destiantion index into the page table.
            loop .loop_3

    ;TODO!!! activate pagging
    activate_paging:
        mov eax, CR4
        or eax, PAGE_ADDRESS_EXTENSION  ; set PAE bit
        mov CR4, eax                    ; activate PAE in CR4
        ; mov eax, PML4E_ADDR
        ; mov CR3, eax					; Load CR3 with the physical address of the PML4
        
    ;TODO!!! transition to 64bits-long mode

    mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        ; Read from the model-specific register.
    or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        ; Write to the model-specific register.

    mov eax, cr0                 ; Set the A-register to control register 0.
    or eax, 1 << 31              ; Set the PG-bit, which is the 31nd bit, and the PM-bit, which is the 0th bit.
    mov cr0, eax                 ; Set control register 0 to the A-register.

jmp 48:.bits64

.bits64:
    [BITS 64]
    mov ax, 40
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

__main:
    ; break
    MOV     RAX, KernelMain     ; after 64bits transition is implemented the kernel must be compiled on x64
    CALL    RAX
    
    ; break
    CLI
    HLT

;;--------------------------------------------------------

__cli:
    CLI
    RET

__sti:
    STI
    RET

__hlt:
	HLT
	RET

__magic:
    XCHG    BX,BX
    RET
    
__enableSSE:                ;; enable SSE instructions (CR4.OSFXSR = 1)  
    [bits 32]
    MOV     EAX, CR4
    OR      EAX, 0x00000200
    MOV     CR4, EAX
    RET
    
__get_intr_flags:
    [bits 64]
    pushf
	BREAK
    pop RAX 
	;MOV [rcx], AX
    ret

__set_intr_flags:
	[bits 64]
	push rcx
	popf
	ret

%macro __pushaq 0
    [bits 64]
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
%endmacro

%macro __popaq 0
    [bits 64]
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro


__IRQ0:
    [bits 64]
    __pushaq
    call IRQ0_handler
    __popaq
    iretq

__IRQ1:
    [bits 64]
    __pushaq
    call IRQ1_handler
    __popaq
    iretq

__IRQ2:
    [bits 64]
    __pushaq
    call IRQ2_handler
    __popaq
    iretq

__IRQ3:
    [bits 64]
    __pushaq
    call IRQ3_handler
    __popaq
    iretq

__IRQ4:
    [bits 64]
    __pushaq
    call IRQ4_handler
    __popaq
    iretq

__IRQ5:
    [bits 64]
    __pushaq
    call IRQ5_handler
    __popaq
    iretq

__IRQ6:
    [bits 64]
    __pushaq
    call IRQ6_handler
    __popaq
    iretq

__IRQ7:
    [bits 64]
    __pushaq
    call IRQ7_handler
    __popaq
    iretq

__IRQ8:
    [bits 64]
    __pushaq
    call IRQ8_handler
    __popaq
    iretq

__IRQ9:
    [bits 64]
    __pushaq
    call IRQ9_handler
    __popaq
    iretq

__ERQ0:
    [bits 64]
    __pushaq
    call ERQ0_handler
    __popaq
    iretq

__ERQ5:
    [bits 64]
    __pushaq
    call ERQ5_handler
    __popaq
    iretq

__ERQ8:
    [bits 64]
    __pushaq
    call ERQ8_handler
    __popaq
    iretq

__ERQ14:
    [bits 64]
    __pushaq
    call ERQ14_handler
    __popaq
    iretq

; FAST_CALL convention 
; return: RAX
__getPAE:
    [bits 64]
    mov RAX, CR4
    ret

; FAST_CALL convention 
; parameter: RCX
__setPAE:
    [bits 64]
    mov CR4, RCX
    ret


EXPORT2C ASMEntryPoint, __cli, __sti, __magic, __enableSSE, __get_intr_flags, __set_intr_flags, __IRQ0, __IRQ1, __IRQ2, __IRQ3, __IRQ4, __IRQ5, __IRQ6, __IRQ7, __IRQ8, __IRQ9, __ERQ0, __ERQ5, __ERQ8, __ERQ14, __hlt


