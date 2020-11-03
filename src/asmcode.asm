.data
var1:	.space	4
var2:	.space	4
.text
.globl main
		move	$fp	$sp
		j	main


func1:
		subi	$sp	$sp	4
#	prtf        1 int  
		li	$a0	1
		li	$v0	1
		syscall
		li	$a0	'\n'
		li	$v0	11
		syscall
#	ret                    
		move	$t0	$ra
		lw	$ra	-4($fp)
		add	$sp	$fp	$zero
		lw	$fp	($fp)
		jr	$t0
#	end               func1


main:
		subi	$sp	$sp	4
#	call  func1            
		sw	$fp	($sp)
		add	$fp	$sp	$0
		subi	$sp	$sp	4
		sw	$ra	($sp)
		jal	func1
		nop
#	end               main
