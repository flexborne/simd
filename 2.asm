section .data
    array_size equ 32 
    alignment equ 32

    add_one dd 1, 1, 1, 1, 1, 1, 1, 1


section .bss
    aligned_mem resq 1

global main

section .text
    extern posix_memalign


main:
    lea rdi, [aligned_mem]
    mov rsi, alignment
    mov rdx, array_size
    mov rax, 0
    call posix_memalign

    mov rdi, [aligned_mem]
    
    vmovdqa ymm0, [rdi]
    vmovdqu ymm1, [add_one]
    vaddpd ymm0, ymm0, ymm1
    vmovdqa [rdi], ymm0   

exit:
    mov rax, 1
    xor rbx, rbx
    int 0x80

initialize_array:
    mov dword [rdi], 1
    mov dword [rdi+4], 2      
    mov dword [rdi+8], 3  
    mov dword [rdi+12], 4      
    mov dword [rdi+16], 5   
    mov dword [rdi+20], 6    
    mov dword [rdi+24], 7 
    mov dword [rdi+28], 8   
    ret