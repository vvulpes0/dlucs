	ld r1,8
	ld r0,1
loop:	call _umul16
	sub r1,1
	jnz loop
done:	j done

_umul16:
	push r1
	push r2
	ld r2,0
mloop:	or r1,r1
	jz mdone
	lsr r1,1
	addc r2,r0
	lsl r0,1
	j mloop
mdone:	ld r0,r2
	pop r2
	pop r1
	ret
