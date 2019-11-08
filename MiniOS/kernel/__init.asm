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

IMPORTFROMC KernelMain

TOP_OF_STACK                equ 0x200000
KERNEL_BASE_PHYSICAL        equ 0x200000
PML4E_ADDR                  equ 0x401000
PDPTE_ADDR                  equ 0x402000
PDE_ADDR                    equ 0x403000
PT_1_ADDR                   equ 0x404000
PT_2_ADDR                   equ 0x405000
PAGE_SIZE                   equ 0x1000  ;4KB

READ_WRITE_PRESENT          equ 0x3
PAGE_ADDRESS_EXTENSION      equ 0b100000
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



    MOV     ESP, TOP_OF_STACK                           ; just below the kernel
    
    break

    ;TODO!!! define page tables; see https://wiki.osdev.org ,Intel's manual, http://www.brokenthorn.com/Resources/

; PD:
;     .limit  dw  PageDirectory.endPD - PageDirectory - 1
;     .base   dd  PageDirectory

; Adresele tabelelor trebuie sa fie aliniate la 1000
; Daca raman in felul asta tabelele de paginare cand se mai dauga cod se vor deplasa
; Mai bine ar fi sa se introduca tabelele la adresa 300000 de exemplu
; ENTER
; PML4:
;     times 512 dq 0x0000000000000002
;     .endPML4:

; PML3:
;     times 512 dq 0
;     .endPML3:

; .loadPageDirectory:
;     MOV EAX, CR4                 ; Set the A-register to control register 4.
;     OR EAX, 1 << 5               ; Set the PAE-bit, which is the 6th bit (bit 5).
;     MOV CR4, EAX                 ; Set control register 4 to the A-register.

;     MOV CR3 [PML4]
;     MOV  0xC0000080
;EXIT

PML4:
    mov QWORD eax, PDPTE_ADDR
    or eax, READ_WRITE_PRESENT
    mov [PML4E_ADDR], eax

PDP:
    mov QWORD eax, PDE_ADDR
    or eax, READ_WRITE_PRESENT
    mov [PDPTE_ADDR], eax

PD:
    mov QWORD eax, PT_1_ADDR
    or eax, READ_WRITE_PRESENT
    mov [PDE_ADDR], eax

    mov QWORD eax, PT_2_ADDR
    or eax, READ_WRITE_PRESENT
    mov [PDE_ADDR+8], eax

;     mov ebx, 0x00000003          ; Set the B-register to 0x00000003.
;     mov ecx, 512                 ; Set the C-register to 512.
;     mov edi, 
 
; .SetEntry:
;     mov DWORD [edi], ebx         ; Set the uint32_t at the destination index to the B-register.
;     add ebx, 0x1000              ; Add 0x1000 to the B-register.
;     add edi, 8                   ; Add eight to the destination index.
;     loop .SetEntry               ; Set the next entry.
; PT_1:
;     %assign     i   0
;     %assign     j   [PT_1]
;     %rep 512  
;     mov QWORD eax, i
;     or eax, READ_WRITE_PRESENT
;     mov j, eax
;     %assign     i   i + PAGE_SIZE
;     %assign     j   j + 8
;     %endrep

; PT_2:
;     %assign     j   [PT_2]
;     %rep 512  
;     mov QWORD eax, i
;     or eax, READ_WRITE_PRESENT
;     mov j, eax
;     %assign     i   i + PAGE_SIZE
;     %assign     j   j + 8
;     %endrep

;!!!!!!!!!!!!!!!!!!!!!!! poate trebuie DWORD in loc de QWORD
PT_1:
    mov eax, 0
    mov edi, PT_1_ADDR
    mov ecx, 512
    .loop_1:
        or eax, READ_WRITE_PRESENT
        mov DWORD [edi], eax
        add eax, PAGE_SIZE
        add edi, 8
        loop .loop_1

PT_2:
    ; mov eax, 0
    mov edi, PT_2_ADDR
    mov ecx, 512
    .loop_2:
        or eax, READ_WRITE_PRESENT
        mov DWORD [edi], eax
        add eax, PAGE_SIZE
        add edi, 8
        loop .loop_2

    ;TODO!!! activate pagging
activate_paging:
    xor eax, eax					; reset eax
    or eax, PAGE_ADDRESS_EXTENSION  ; set PAE bit
    mov CR4, eax                    ; activate PAE in CR4
	mov eax, PML4E_ADDR
    mov CR3, eax					; Load CR3 with the physical address of the PML4
    
    ; ;TODO!!! transition to 64bits-long mode
    ; [BITS 64]

    mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        ; Read from the model-specific register.
    or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        ; Write to the model-specific register.

    mov eax, cr0                 ; Set the A-register to control register 0.
    or eax, 1 << 31 | 1 << 0     ; Set the PG-bit, which is the 31nd bit, and the PM-bit, which is the 0th bit.
    mov cr0, eax                 ; Set control register 0 to the A-register.

    mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        ; Read from the model-specific register.
    or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        ; Write to the model-specific register.

    mov eax, cr0                 ; Set the A-register to control register 0.
    or eax, 1 << 31              ; Set the PG-bit, which is the 32nd bit (bit 31).
    mov cr0, eax                 ; Set control register 0 to the A-register.

jmp 48:__main

__main:
;[bits 64]
    MOV     EAX, KernelMain     ; after 64bits transition is implemented the kernel must be compiled on x64
    CALL    EAX
    
    break
    CLI
    HLT

;;--------------------------------------------------------

__cli:
    CLI
    RET

__sti:
    STI
    RET

__magic:
    XCHG    BX,BX
    RET
    
__enableSSE:                ;; enable SSE instructions (CR4.OSFXSR = 1)  
    MOV     EAX, CR4
    OR      EAX, 0x00000200
    MOV     CR4, EAX
    RET
    
EXPORT2C ASMEntryPoint, __cli, __sti, __magic, __enableSSE


