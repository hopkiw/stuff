mov    ax,0x12
add    ax,0x2
mov    bx,0x13
sub    bx,0x13
mov    cx,0x14
cmp    cx,0x13
call   0x7f000013
push   cx
mov    dx,0x15
pop    dx
mov    ax,0x00
mov    bx,0x00
add    ax,0x01
cmp    ax,0x05
jne    0x7f00000b
mov    [cx],ax
mov    ax,bx
mov    bx,ax
nop
mov    bx,0x13
ret    fake
nop
