.data
    prompt: .asciiz "Enter par:"
    name_prompt: .asciiz "Enter player name:"
    #player: .space 64
    nln: .asciiz "\n"
    done: .asciiz "DONE"
    plus: .asciiz "+"
    space: .asciiz " "

.text
.align 2
.globl main

strcmp:
    # Load current characer from the string
    # (In addition to being the start of the function, this is also a loop)
    lb $t0, 0($a0)
    lb $t1, 0($a1)
    # If the characters differ, we are done looping
    bne $t0, $t1, done_with_strcmp_loop
    # Otherwise, increment and go back to the start, unless we’ve reached NULL
    addi $a0, $a0, 1
    addi $a1, $a1, 1
    bnez $t0, strcmp
    # If characters differ or we’ve reached the end, return
done_with_strcmp_loop:
    sub $v0, $t0, $t1
    jr $ra

strcpy:
    _strcpy_loop:
    # Copy the character from the source to the destination
    lb $t0, 0($a0)
    sb $t0, 0($a1)
    beqz $t0, _strcpy_done
    addi $a0, $a0, 1
    addi $a1, $a1, 1
    b _strcpy_loop
_strcpy_done:
    jr $ra


compare_stat:
    addi $sp, $sp, -24
    sw $ra, 0($sp)
    sw $s0, 4($sp)
    sw $s1, 8($sp)
    sw $s3, 12($sp)
    sw $s4, 16($sp)
    sw $s5, 20($sp)



    beq $a0, $zero, _true_case
    beq $a1, $zero, _false_case

    move $s0, $a0
    move $s1, $a1

    lw $s3, 64($s0)  # Changed offset to 64 for stat
    lw $s4, 64($s1)  # Changed offset to 64 for stat
    bgt $s3, $s4, _true_case
    blt $s3, $s4, _false_case
    #check equal case)
    move $a0, $s0
    move $a1, $s1

    # save t registers
    addi $sp, $sp, -8
    sw $t0, 0($sp)
    sw $t1, 4($sp)

    # call strcmp
    jal strcmp

    # collapse stack frame
    lw $t0, 0($sp)
    lw $t1, 4($sp)
    addi $sp, $sp, 8

    # move output
    move $s5, $v0
    bgtz $s5, _true_case 
    j _false_case

_true_case:
    li $v0, 1 
    j finish             

_false_case:
    li $v0, 0  

finish:
    lw $ra, 0($sp)
    lw $s0, 4($sp)
    lw $s1, 8($sp)
    lw $s3, 12($sp)
    lw $s4, 16($sp)
    lw $s5, 20($sp)
    addi $sp, $sp, 24           
    jr $ra   

insert:
    addi $sp,$sp, -20
    sw $ra, 0($sp)
    sw $s0, 4($sp)
    sw $s1, 8($sp)
    sw $s2, 12($sp)
    sw $s3, 16($sp)

    #beqz $v0, insert_at_head
    move $s0, $a0  # head
    move $s1, $a1  # toInsert

    jal compare_stat 
    bnez $v0, insert_at_head

    move $s2, $s0  # current = head

while_condition:
    lw $s3, 68($s2)     # Load current->next
    beqz $s3, insert_at_end

    move $a0, $s3       # a0 = current->next
    move $a1, $s1       # a1 = toInsert
    jal compare_stat    
    bgtz $v0, insert_between  # If current->next > toInsert, insert between

    #beqz $v0, insert_between    
    move $s2, $s3       # current = current->next
    j while_condition

insert_at_head:
    sw $s0, 68($s1)     # Set toInsert->next = head
    move $v0, $s1       # Update head to be toInsert
    j finish_insert

insert_between:
    lw $t1, 68($s2)     # Load current->next
    sw $t1, 68($s1)     # Set toInsert->next = current->next
    sw $s1, 68($s2)     # Set current->next = toInsert
    move $v0, $s0       # Return the original head
    j finish_insert

insert_at_end:
    sw $zero, 68($s1)   # Set toInsert->next = NULL
    sw $s1, 68($s2)     # Set current->next = toInsert
    move $v0, $s0       # Return the original head
    j finish_insert

finish_insert:
    lw $ra, 0($sp)
    lw $s0, 4($sp)
    lw $s1, 8($sp)
    lw $s2, 12($sp)
    lw $s3, 16($sp)
    addi $sp, $sp, 20
    jr $ra

printList:
    move $t0, $a0  # Save the head pointer

print_loop:
    beqz $t0, print_end

    # Print player name
    li $v0, 4
    move $a0, $t0
    syscall

    # Print newline
    li $v0, 4
    la $a0, space
    syscall
    lw $t1, 64($t0)

    blez $t1, print_neg
print_pos:
    li $v0, 4
    la $a0, plus
    syscall
    j print_stat

print_neg:
    j print_stat

print_stat:
    # print stat int 
    li $v0, 1
    lw $a0, 64($t0)
    syscall

    # Print newline
    li $v0, 4
    la $a0, nln
    syscall

    # Move to next player
    lw $t0, 68($t0)
    j print_loop

print_end:
    jr $ra

main:
    addi $sp, $sp, -20
    sw $ra, 0($sp)
    sw $s0, 4($sp)
    sw $s1, 8($sp)
    sw $s2, 12($sp)
    sw $s3, 16($sp)

    li $s0, 0  # head = NULL

    # Prompt for par
    li $v0, 4
    la $a0, prompt
    syscall

    # Read par
    li $v0, 5
    syscall
    move $s1, $v0  # Store par in $s1

loop:
    # Allocate memory for new player
    li $v0, 9
    li $a0, 72
    syscall
    move $s2, $v0

    li $v0, 9
    li $a0, 64
    syscall
    move $s3, $v0

    # Prompt for player name
    li $v0, 4
    la $a0, name_prompt
    syscall

    # Read player name
    li $v0, 8
    move $a0, $s3
    li $a1, 64
    syscall


    # Remove newline character
    move $t0, $a0
remove_newline:
    lb $t1, 0($t0)
    beqz $t1, newline_removed
    bne $t1, 10, next_char
    sb $zero, 0($t0)
    j newline_removed
next_char:
    addi $t0, $t0, 1
    j remove_newline
newline_removed:

    # Check if input is "DONE"
    move $a0, $s3
    la $a1, done
    jal strcmp
    beqz $v0, done_reading

    # Read stat
    li $v0, 5
    syscall
    sub $t1, $v0, $s1  # Calculate difference from par
    sw $t1, 64($s2)    # Store stat in new player struct

### added this code to make sure to string copy ------------------------------------
    # Copy player name to new_player->player (the start of the allocated block)
    move $a0, $s3             # Load address of player buffer
    move $a1, $s2              # $t1 points to the start of the struct (player name field is at offset 0)
    jal strcpy                 # Copy the player's name into the struct

    # Set new_player->next to NULL (0)
    li $t5, 0                  # Load 0 for NULL
    sw $t5, 68($s2)            # Store NULL in new_player->next (offset 68)
###### this broke the entire thing ----------------------------------------------------

    # Insert new player
    move $a0, $s0
    move $a1, $s2
    jal insert
    move $s0, $v0  # Update head

    j loop

done_reading:
    # Print the list
    move $a0, $s0
    jal printList

    # Restore stack and return
    lw $ra, 0($sp)
    lw $s0, 4($sp)
    lw $s1, 8($sp)
    lw $s2, 12($sp)
    lw $s3, 16($sp)
    addi $sp, $sp, 20
    jr $ra
