	ld    r2,8
	ld    r0,1
loop:	ld    r1,r2
	call  _umul16x16
	sub   r2,1
	jnz   loop
done:	j     done

	;; multiply r0 by r1, store into r1:r0 (hi:lo)
_umul16x16:
	push r2
	push r3
	push r4

	ld r2,0
	ld r3,0
	or r1,r1
mloop:	jz mdone
	asr r1,1
	addc r2,r0
	adcc r3,r4
	lsl r0,1
	rlc r4,1
	or r1,r1
	j mloop
mdone:
	ld r0,r2
	ld r1,r3

	pop r4
	pop r3
	pop r2
	ret
