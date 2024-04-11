############################################################################
#                       Lab 3 - Pre-Lab
#                       EC413
#
#    		Assembly Language Lab -- Addressing.
#
############################################################################
#  DATA
############################################################################
        .data           # Data segment
Hello:	.asciiz " \n Hello World! \n "  # declare a zero terminated string
Q1:		.asciiz " \n Q1 Answer: \n"	
Q2:		.asciiz " \n Q2 Answer: \n"
In1: .word	5		# Input data for Question 1 
In2: .word	9		# Input data for Question 1
In3: .word	7		# Input data for Question 1
Out1:.word 0		# Output data for Question 1
In4: .word 4, 7, 9	# Input data for Question 2
Out2:.word 0, 0, 0	# Output data for Question 2
space:	.asciiz	" "	# declare a zero terminate string

############################################################################
#  CODE
############################################################################
        .text                   # code segment
#
# Print out greetings
#
main:
        la	$a0,Hello		# address of string to print
        li	$v0,4			# system call code for print_str
        syscall         	# print the string
		
#
# Question 1-- 
# Use direct addressing to add 3 numbers in memory location In1, In2, In3 and write the result to Out1
# Print out the value stored in Out1 after calculation
#
##############Signify the output of Q1############
		la	$a0,Q1
        li	$v0,4	
        syscall
		
##############Your code from here#################

        la  $t0,In1
        lw  $t0,0($t0)
        la  $t1,In2
        lw  $t1,0($t1)
        la  $t2,In3
        lw  $t2,0($t2)

        add $t0,$t0,$t1
        add $t0,$t0,$t2

        la  $t3,Out1
        sw  $t0,0($t3)

##############Print out Out1######################			
		lw	$a0,Out1		# Load a value in Out1 to $a0
		li	$v0,1			# syscall command 1 -- Print out 1 word
		syscall
		
		
		
		
		

#
# Question 2--
# Use indirect addressing to reverse the data sequence in array In4 and store the new sequence in Out2
# Print out the value stored in Out2 after operation
#

##############Signify the output of Q2############
		la	$a0,Q2
        li	$v0,4	
        syscall
		
##############Your code from here#################		
		
		la  $t4,In4
        la  $t5,Out2

        lw  $t6,0($t4)
        sw  $t6,8($t5)
        lw  $t6,4($t4)
        sw  $t6,4($t5)
        lw  $t6,8($t4)
        sw  $t6,0($t5)
				
		
##############Print out Out2######################		
		li	$t6,3
		la	$t5,Out2		# $t5 point to the beginning of Out2
Print_Out2:
		lw	$a0,($t5)		# Load the first value in Out2 to $a0
		li	$v0,1			# syscall command 1 -- Print out 1 word
		syscall
		add $t5,4			# $t5 point to the 2nd element of Out2
		add $t6,-1			# counter--
		bgtz $t6,Print_Out2		
		
		
		
		
    jr $ra		
		
		