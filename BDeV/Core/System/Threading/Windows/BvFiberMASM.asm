.code

; void SaveFPControlASM(const void* pFiber);
; rcx: const void* pFiber
SaveFPControlASM PROC
    ; save MMX control- and status-word
    stmxcsr  [rcx+0a0h]
    ; save x87 control-word
    fnstcw  [rcx+0a4h]

    ret
SaveFPControlASM ENDP

; void StartFiberASM(const void* pFiber);
; rcx: const void* pFiber
StartFiberASM PROC FRAME
    .endprolog
    ; When StartFiberASM is called, here's what the registers have:
    ; * rdi: Pointer to the fiber's argument
    ; * rsi: Address to the fiber's entry point function
    ; * rbp: Address to the fiber's exit point function
    
    ; We pass the fiber's argument to the rcx register for the next function
    mov rcx, rdi

    ; Push the fiber's exit point function address
    ; This also helps keeping the stack aligned
    push rbp

    ; Jump to the fiber's entry point function address
    jmp rsi
StartFiberASM ENDP

; void SwitchToFiberASM(void** pSrcFiber, const void* pDstFiber);
; rcx: void** pSrcFiber
; rdx: const void* pDstFiber
SwitchToFiberASM PROC FRAME
    .endprolog
    ; prepare stack
    lea rsp, [rsp-0118h]

    ; save XMM registers
    movaps  [rsp], xmm6
    movaps  [rsp+010h], xmm7
    movaps  [rsp+020h], xmm8
    movaps  [rsp+030h], xmm9
    movaps  [rsp+040h], xmm10
    movaps  [rsp+050h], xmm11
    movaps  [rsp+060h], xmm12
    movaps  [rsp+070h], xmm13
    movaps  [rsp+080h], xmm14
    movaps  [rsp+090h], xmm15
    ; save MMX control- and status-word
    stmxcsr  [rsp+0a0h]
    ; save x87 control-word
    fnstcw  [rsp+0a4h]

    ; load NT_TIB from the global segment and save
    ; its values in the stack
    mov  r10,  gs:[030h]
    ; save fiber local storage
    mov  rax, [r10+020h]
    mov  [rsp+0b0h], rax
    ; save current deallocation stack
    mov  rax, [r10+01478h]
    mov  [rsp+0b8h], rax
    ; save current stack limit
    mov  rax, [r10+010h]
    mov  [rsp+0c0h], rax
    ; save current stack base
    mov  rax,  [r10+08h]
    mov  [rsp+0c8h], rax

    mov [rsp+0d0h], r12  ; save R12
    mov [rsp+0d8h], r13  ; save R13
    mov [rsp+0e0h], r14  ; save R14
    mov [rsp+0e8h], r15  ; save R15
    mov [rsp+0f0h], rdi  ; save RDI
    mov [rsp+0f8h], rsi  ; save RSI
    mov [rsp+0100h], rbx  ; save RBX
    mov [rsp+0108h], rbp  ; save RBP
    
    ; update the RSP in the first argument
    mov [rcx], rsp

    ; load the new RSP from RDX
    mov  rsp, rdx

    ; load XMM storage
    movaps  xmm6, [rsp]
    movaps  xmm7, [rsp+010h]
    movaps  xmm8, [rsp+020h]
    movaps  xmm9, [rsp+030h]
    movaps  xmm10, [rsp+040h]
    movaps  xmm11, [rsp+050h]
    movaps  xmm12, [rsp+060h]
    movaps  xmm13, [rsp+070h]
    movaps  xmm14, [rsp+080h]
    movaps  xmm15, [rsp+090h]
    ; restore MMX control- and status-word
    ldmxcsr  [rsp+0a0h]
    ; save x87 control-word
    fldcw   [rsp+0a4h]

    ; load NT_TIB from the global segment and save
    ; the stack values in it
    mov  r10,  gs:[030h]
    ; restore fiber local storage
    mov  rax, [rsp+0b0h]
    mov  [r10+020h], rax
    ; restore current deallocation stack
    mov  rax, [rsp+0b8h]
    mov  [r10+01478h], rax
    ; restore current stack limit
    mov  rax, [rsp+0c0h]
    mov  [r10+010h], rax
    ; restore current stack base
    mov  rax, [rsp+0c8h]
    mov  [r10+08h], rax

    mov r12, [rsp+0d0h] ; restore R12
    mov r13, [rsp+0d8h] ; restore R13
    mov r14, [rsp+0e0h] ; restore R14
    mov r15, [rsp+0e8h] ; restore R15
    mov rdi, [rsp+0f0h] ; restore RDI
    mov rsi, [rsp+0f8h] ; restore RSI
    mov rbx, [rsp+0100h] ; restore RBX
    mov rbp, [rsp+0108h] ; restore RBP

    ; prepare stack
    lea rsp, [rsp+0118h]

    ret
SwitchToFiberASM ENDP

END