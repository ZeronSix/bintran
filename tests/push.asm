section .text
        global _start
start:
        push    12345678h
        add     rsp, 4

        mov eax, [rsp+123456FFh]
        push rax
