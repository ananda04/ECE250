.text
    # test boolean logic instrctions

    addi $r1, $r0, 28		# 0x001c                        28
    addi $r2, $r0, 26		# 0x001a                        26

    not $r3, $r1            # ~0x001c == 0xFFE3             ~28 == 65507
    not $r4, $r2            # ~0x001a == 0xFFE5             ~26 == 65509

    xor $r5, $r1, $r2       # 0x001c & 0x001a == 0x0018     28 ^ 26 == 6
    xor $r6, $r3, $r4       # 0xFFE3 & 0xFFE5 == 0xFFE1     65507 ^ 65509 == 6
    
    not $r7, $r6            # ~0x0006 == 0xFFF9             ~6 == 30

    halt

.data
