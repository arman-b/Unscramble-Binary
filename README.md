# Unscrambler

Exercise for a Reverse Engineering Lab

You've been hired to implement the descrambling section of a
highly-secure copyright-protection system.  Your program will be given
two files: a scambled piece of content, and a descrambling script.
Your program should descramble the content file in place on the
filesystem.

The descrambling script contains a series of operations which must be
performed to descramble the content.  Each line in the script contains
one operation, and looks like the following:

<operation> <size> <offset> <constant>

The four fields in the operation should be interpreted as follows:

operation: the operation to perform.  Possible operations are:
  - ADD (Add a constant to the value in the file)
  - SUB (Subtract a constant from the value in the file)
  - XOR (Bitwise XOR a constant with the value in the file)
  - WRITE (Write a constant value to the position in the file)

size: the data size to operate on.  Possible sizes are:
  - BYTE (1 byte)
  - WORD (2 bytes)
  - DWORD (4 bytes)
  
For mathematical operations, values in the file should be treated as
little-endian numbers.

offset: The position in the file where the operation should start.
Encoded as a hexadecimal number.  For the ADD, SUB and XOR operations,
the first operand should be loaded from this offset in the file, and
the result should be written back to this offset.

constant: The second operand for ADD, SUB and XOR, and only operand
for WRITE.  Encoded as a hexadecimal number.


For example, the following are all valid lines in a decoding script:

XOR BYTE 0x12 0x27
WRITE WORD 0x28 0xFFFE
ADD DWORD 0x1780 0x78242318

All operations should be performed using unsigned arithmetic, ignoring overflow.


As an example, suppose the scrambled file has contents:

00 65 6c 6c 08 db 33 6e 72 6c 64

and the decryption script is:

WRITE BYTE 0x00 0x48
ADD DWORD 0x4 0x01234567

First, we'd write the value 0x48 at offset 0x00, yielding

48 65 6c 6c 08 db 33 6e 6f 72 6c 64

Then, we'd load a four-byte unsigned value from offset 0x04
(0x6e33db08), and add 0x01234567, which results in 0x6f57206f.
Writing that back into the file yields:

48 65 6c 6c 6f 20 57 6f 72 6c 64

("Hello World" in ASCII).
