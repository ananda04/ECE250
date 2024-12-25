.data
    prompt: .asciiz ":"
    result: .asciiz ""

.text
.align 2
.globl main

main:

# open stack frame this includes s0-3 and ra
addi $sp, $sp, -4
sw $ra, 0($sp)

# prompt user 
li $v0, 4
la $a0, prompt
syscall

# read user arguement 
li $v0, 5
syscall
move $a0, $v0

# go to recurse 
jal recurse

# result prompt
li $v0, 4
la $a0, result
syscall 

# read user result 
move $a0, $v1
li $v0, 1
syscall 

# clean up stack frame 
lw $ra, 0($sp)
addi $sp, $sp, 4

# return register clean up
jr $ra

recurse:
# open stack frame 
addi $sp, $sp, -20
sw $ra, 0($sp)
sw $s0, 4($sp)
sw $s1, 8($sp)
sw $s2, 12($sp)
sw $s3, 16($sp)

# base case if statement 
li $t0, 0
beq $a0, $t0, _base

# multiply 2 to N+1
li $s0, 2
addi $s2, $a0, 1
mul $s2, $s2, $s0

#save current value of $a0 in $s3
move $s3, $a0

# operation done before recurse + recurse
addi $a0, $a0, -1
jal recurse 

# 3 * recurse(n-1)
li $s1, 3                  
mul $v1, $v1, $s1           

# subtract 17 from final value
add $s3, $v1, $s2
addi $s3, $s3, -17

move $v1, $s3
j _end

_base:

# load two to return register
li $v1, 2
j _end 

_end:


# close stack frame 
lw $ra, 0($sp)
lw $s0, 4($sp)
lw $s1, 8($sp)
lw $s2, 12($sp)
lw $s3, 16($sp)
addi $sp, $sp, 20

# jr ra 
jr $ra



