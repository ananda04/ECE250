.data
    prompt: .asciiz ":"
    result: .asciiz ":" 

.text
.align 2

.globl main

main:
    # open stack frame
    addi $sp, $sp, -4
    sw $ra, 0($sp)

    #prompt value and syscall
    li $v0, 4
    la $a0, prompt
    syscall

    #read value
    li $v0, 5
    syscall
    move $a0, $v0

    #loop through n times using input agruement 
    li $t0, 0
    li $t1, 3
    li $t2, 1
loop:
    bge $t0, $a0, _end
    mul $t2, $t2, $t1
    addi $t0, $t0, 1
    j loop
_end:
    # subtract 3
    addi $t3, $t2, -3

    #prompt result 
    li $v0, 4
    la $a0, result
    syscall

    #read result
    move $a0, $t3
    li $v0, 1
    syscall 

    #collapse stack frame
    lw $ra, 0($sp)
    addi $sp, $sp, 4

    # call jr
    jr $ra


