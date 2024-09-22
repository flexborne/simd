section .data
    arr1 dd 0, 1, 2, 3, 8, 10, 41, 3
    arr2 dd 5, 7, 8, 10, 8, 88, 21, 33
    len equ ($ - arr1) / 4

section .bss
    result resd len

section .text
    global _start

; Input: rdi = pointer to array1, rsi = pointer to array2, rdx = pointer to result
add_arrays:
    vmovdqa ymm0, [rdi]
    vmovdqa ymm1, [rsi]
    vaddpd ymm0, ymm0, ymm1
    vmovdqa [rdx], ymm0

    ret

_start:
    lea rdi, [arr1]
    lea rsi, [arr2]
    lea rdx, [result]

    call add_arrays

exit:
    mov rax, 1
    xor rbx, rbx
    int 0x80