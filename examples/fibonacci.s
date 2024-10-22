	ld    r0,0
	ld    r1,1
loop:	add   r0,r1
	xcg   r0,r1
	offs  r6,loop
	offsc r6,done
done:	j     r6
