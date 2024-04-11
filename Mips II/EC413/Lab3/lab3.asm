############################################################################
#                       Lab 3
#                       EC413
#
#    		Assembly Language Lab -- Programming with Loops.
#
############################################################################
#  DATA
############################################################################
        .data           # Data segment
Hello:  .asciiz " \n Hello World! \n "  # declare a zero terminated string
Hello_len: .word 16
AnInt:	.word	12		# a word initialized to 12
space:	.asciiz	" "	        # declare a zero terminate string
WordAvg:   .word 0		# use this variable for part 6
ValidInt:  .word 0		#
ValidInt2: .word 0		#
lf:     .byte	10, 0		# string with carriage return and line feed
InLenW:	.word   4       	# initialize to number of words in input1 and input2
InLenB:	.word   16      	# initialize to number of bytes in input1 and input2
        .align  4
Input1:	.word	0x01020304,0x05060708
	.word	0x090A0B0C,0x0D0E0F10
        .align  4
Input2: .word   0x01221117,0x090b1d1f   # input
        .word   0x0e1c2a08,0x06040210
        .align  4
Copy:  	.space  0x80    # space to copy input word by word
        .align  4
 
Enter: .asciiz "\n"
Comma: .asciiz ","
Empty: .space 12	# put some empty spaces here so the starting addr of Text is clear
Text: .asciiz " \n It is exciting to watch flying fish after a hard day of work. I do not know why some fish prefer flying and other fish would rather swim. It seems like the fish just woke up one day and decided, Hey, today is the day to fly away \n "	# a big string
############################################################################
#  CODE
############################################################################
        .text                   # code segment
#
# print out greeting
#
main:
        la	$a0,Hello	# address of string to print
        li	$v0,4		# system call code for print_str
        syscall                 # print the string

	
#Code for Item 2
#Count number of occurrences of letter "a" and "e" in Text string and compute the difference between the number of occurrences


        la   $t0, Text
        li   $t1, 0    # loop var
        li   $t2, 0    # count of a
        li   $t3, 0    # count of e

count:  lb   $t4, 0($t0)   #load a byte
        beq  $t4, 0, done  #if null byte, we are done
        beq  $t4, 97, pa     #if a, increment a count
        beq  $t4, 101, pe    #if e, increment e count
        addi $t0, $t0, 1    #increment pointer
        j    count          #repeat loop

pa:      addi $t2, $t2, 1    #increment a count
        addi $t0, $t0, 1    #increment pointer
        j    count          #repeat loop

pe:      addi $t3, $t3, 1    #increment e count
        addi $t0, $t0, 1    #increment pointer
        j    count          #repeat loop


done:   sub  $t5, $t2, $t3  #compute difference
        li   $v0, 1          #print integer
        move $a0, $t5
        syscall




################################################################################
        la	$a0,Enter	# address of string to print
        li	$v0,4		# system call code for print_str
        syscall                 # print the string
################################################################################

#
# Code for Item 3 -- 
# Print the integer value of numbers from 5 and less than AnInt
#


        li  $t0, 5
        li  $t1, 0  # loop var
        li  $t2, 12  # AnInt

loop:   bge $t0, $t2, done2
        li  $v0, 1
        move $a0, $t0
        syscall
        la  $a0, space
        li  $v0, 4
        syscall
        addi $t0, $t0, 1
        j   loop

done2: xor $t1, $t1, $t1       # don't do anything



###################################################################################
        la	$a0,Enter	# address of string to print
        li	$v0,4		# system call code for print_str
        syscall                 # print the string
###################################################################################
#
# Code for Item 4 -- 
# Print the integer values of each byte less than 4 in the array Input2 (with spaces)
#


        la  $t0, Input2
        li  $t1, 0  # loop var
        li  $t2, 16  # length of Input2 (4*4)
        li  $t4, 4

loop2:  bge $t1, $t2, done3
        lb  $t3, 0($t0)
        blt $t3, $t4, toprint
        addi $t0, $t0, 1
        addi $t1, $t1, 1
        j   loop2

toprint:  li  $v0, 1
        move $a0, $t3
        syscall               # print the integer
        la  $a0, space
        li  $v0, 4
        syscall               # print space
        addi $t0, $t0, 1
        addi $t1, $t1, 1
        j   loop2

done3: xor $t1, $t1, $t1       # don't do anything




###################################################################################
#
# Code for Item 5 -- 
# Write code to copy the contents of Input2 to Copy
#


        la  $t0, Input2
        la  $t1, Copy
        li  $t2, 16  # length of Input2 (I want to loop 4 times, but I increament by 4 each time)
        li  $t5, 0   # loop var
loop3:  beq $t5, $t2, done4
        lw  $t3, 0($t0)
        sw  $t3, 0($t1)

        addi $t0, $t0, 4
        addi $t1, $t1, 4
        addi $t5, $t5, 4
        j   loop3

done4:  





#################################################################################
        la	$a0,Enter	# address of string to print
        li	$v0,4		# system call code for print_str
        syscall                 # print the string
###################################################################################
#
# Code for Item 6 -- 
# Print the integer average of squares of the contents of array Input1
#

        la  $t0, Input1
        li  $t1, 0  # loop var
        li  $t2, 16  # length of Input1
loop4:  beq $t1, $t2, done5
        
        lb  $t3, 0($t0)
        mul $t3, $t3, $t3
        add $t4, $t4, $t3

        addi $t0, $t0, 1
        addi $t1, $t1, 1
        j   loop4
done5:  
        div $t4, $t2
        mflo $t4
        li   $v0, 1
        move $a0, $t4
        syscall    # print the integer  






#################################################################################
        la	$a0,Enter	# address of string to print
        li	$v0,4		# system call code for print_str
        syscall                 # print the string
##################################################################################
#
# Code for Item 7 -- 
# Display the first 10 integers that are divisible by either 7 and 13 (with space)
#



        li  $t0, 0  # loop var
        li  $t1, 0  # number to check

        li $t2, 7
        li $t3, 13

loop5:  bge $t0, 10, done6

        div $t1, $t2
        mfhi $t4
        div $t1, $t3
        mfhi $t5

        beq $t4, 0, print2
        beq $t5, 0, print2

        addi $t1, $t1, 1
        j   loop5

print2: li  $v0, 1
        move $a0, $t1
        syscall               # print the integer
        la  $a0, space
        li  $v0, 4
        syscall               # print space
        addi $t1, $t1, 1
        addi $t0, $t0, 1
        j   loop5

done6:        # don't do anything


#################################################################################
        la	$a0,Enter	# address of string to print
        li	$v0,4		# system call code for print_str
        syscall                 # print the string
##################################################################################



#
# Code for Item 8 -- 
# Repeat step 7 but display the integers in 3 lines each with 5 integers (with spaces between the integers)
# This must be implemented using a nested loop.
#

        li $t0, 0  # loop var
        li $t1, 0  # number to check
        li $t2, 7
        li $t3, 13
        li $t4, 0  #line
        li $t5, 0  #integers in line

loop6:  beq $t4, 3, Exit

loop7:  beq $t5, 5, penter

        div $t1, $t2
        mfhi $t6
        div $t1, $t3
        mfhi $t7

        beq $t6, 0, print3
        beq $t7, 0, print3

        addi $t1, $t1, 1

        j   loop6


print3: li  $v0, 1
        move $a0, $t1
        syscall               # print the integer
        la  $a0, space
        li  $v0, 4
        syscall               # print space
        addi $t1, $t1, 1
        addi $t5, $t5, 1
        j   loop6


penter: la  $a0, Enter
        li  $v0, 4
        syscall
        addi $t4, $t4, 1
        li  $t5, 0
        j   loop6

Exit:
	jr $ra