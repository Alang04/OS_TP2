GLOBAL getSeconds
GLOBAL getMinutes
GLOBAL getHour
GLOBAL getDayOfMonth
GLOBAL getMonth
GLOBAL getYear
GLOBAL inb
GLOBAL outb
GLOBAL acquireLock
GLOBAL releaseLock

section .text
	
getSeconds:
	mov al, 0
	out 0x70, al
	in al, 0x71
	ret

getMinutes:
	mov al, 2
	out 0x70, al
	in al, 0x71
	ret

getHour:
	mov al, 4
	out 0x70, al
	in al, 0x71
	ret

getMonth:
	mov al, 8
	out 0x70, al
	in al, 0x71
	ret

getYear:
	mov al, 9
	out 0x70, al
	in al, 0x71
	ret

getDayOfMonth:
	mov al, 7
	out 0x70, al
	in al, 0x71
	ret

outb:
	push rbp
	mov rbp, rsp

	mov dx, di
	mov al, sil
	out dx, al

	pop rbp
	ret

inb:
	push rbp	
	mov rbp, rsp

	mov dx, di 
	in al, dx 

	pop rbp
	ret

; void acquireLock(uint8_t * lock)
; Spinlock: xchg es atomico (lleva LOCK implicito). Sale cuando el valor previo era 0
acquireLock:
	mov al, 1
.retry:
	xchg al, [rdi]
	test al, al
	jz .acquired
	pause
	jmp .retry
.acquired:
	ret

; void releaseLock(uint8_t * lock)
releaseLock:
	mov byte [rdi], 0
	ret
