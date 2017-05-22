START:
; HALT
        ret

; PUSH
        push 0x12345678

; POP
        pop rax

; ADD
jmp_test3:
        pop rax  ; load to rax from stack
        pop r8   ; load to rax

        mov rax, r9
        mov rax, r14
        mov r8, r14
        mov r8, r9

        add rax, r8

        push rax

        mov r9, rax
        mov r14, rax


; SUB
        pop r8
        pop rax
        sub rax, r8
        sub r8, rax
        mov r8, rax
        push rax

; MUL
        pop rax
        pop r8
        imul rax, r8
        push rax

; DIV
jmp_test:
        pop r8
        pop rax
        idiv r8
jmp_test2:
        push rax

; JMP
        jmp jmp_test
        jmp jmp_test2
        jmp jmp_test3
        jmp kek

; JMC
        pop rax
        cmp rax, 0
        jne START
kek:
        jnz jmp_test

; GZ
        pop rax
        mov r8, 1
        cmp rax, 0
        mov rax, 0
        cmovg rax, r8
        push rax

; GEZ
        pop rax
        mov r8, 1
        cmp rax, 0
        mov rax, 0
        cmovge rax, r8
        push rax

; BZ
        pop rax
        mov r8, 1
        cmp rax, 0
        mov rax, 0
        cmovl rax, r8
        push rax

; BEZ
        pop rax
        mov r8, 1
        cmp rax, 0
        mov rax, 0
        cmovle rax, r8
        push rax

; STORE
        xor rax, rax
        pop rax
        mov [r10+0x12345678], rax
        mov [r10-0x12345678], rax

; LOAD
        xor rax, rax
        mov eax, [r10+0x12345678]
        push rax

; HEADER
        mov r10, rsp
        sub r10, 8
        mov r11, rdi
        mov r12, rsi
        mov r13, rsp

; NEQZ and EQZ
        cmove rax, r8
        cmovne rax, r8

; INPUT
        xor rax, rax
        push r10
        push r11
        call r11
        pop r11
        pop r10
        push rax

; OUTPUT
        pop rdi
        call r12

; FOOTER
        mov rsp, r13
