section .data
    array_size equ 32 
    alignment equ 32

section .bss
    aligned_mem1 resq 1
    aligned_mem2 resq 2

global main

section .text
    extern posix_memalign
    extern free

main:
    lea rdi, [aligned_mem1]
    mov rsi, alignment
    mov rdx, array_size
    mov rax, 0
    call posix_memalign

    lea rdi, [aligned_mem2]
    mov rsi, alignment
    mov rdx, array_size
    mov rax, 0
    call posix_memalign
    
    mov rdi, [aligned_mem1]
    call initialize_array

    mov rdi, [aligned_mem2]
    call initialize_array

    mov rdi, [aligned_mem1]
    mov rsi, [aligned_mem2]

    vmovdqa ymm0, [rdi]
    vmovdqa ymm1, [rsi]

    vmulpd ymm0, ymm0, ymm1

    vextractf128 xmm2, ymm0, 1  
    vpaddd xmm0, xmm0, xmm2      

    mov rdi, [aligned_mem1]   
    call free       
    mov rdi, [aligned_mem2]   
    call free               

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