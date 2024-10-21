	;; sample reset code : must be at address 0
reset:
	ld sp,$8000
	neg r0,0     ; set condition codes to a consistent state
	j sp         ; go to beginning of progROM

	;; sample interrupt handler : must be at address 8
interrupt:
	;; save register r0 and condition codes
	push r0
	flg r0,r0
	push r0
	;; do something useful
	;; ...
	;; restore condition codes and register r0
	pop r0
	flg r0,r0
	pop r0
	ret
