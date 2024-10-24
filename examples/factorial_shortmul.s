	ld r1,8
	ld r0,1
loop:	call _umul16
	sub r1,1
	jnz loop
done:	j done

	;; multiply r0 by r1, store into r0
_umul16:
	push r1
	push r2
	push r3
	ld r2,0
mloop:	or r1,r1
	jz mdone
	ctz r3,r1
	lsl r0,r3
	add r2,r0
	add r3,1
	lsr r1,r3
	lsl r0,1
	j mloop
mdone:	ld r0,r2
	pop r3
	pop r2
	pop r1
	ret
