#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# This exploit template was generated via:
# $ pwn template --host 127.0.0.1 --port 1111
from pwn import *
from pwn import p64,u64
# Set up pwntools for the correct architecture
context.update(arch='i386')
exe = context.binary = ELF('chaos')
import subprocess

# Many built-in settings can be controlled on the command-line and show up
# in "args".  For example, to dump all data sent/received, and disable ASLR
# for all created processes...
# ./exploit.py DEBUG NOASLR
# ./exploit.py GDB HOST=example.com PORT=4141 EXE=/tmp/executable
host = args.HOST or '5.1.53.43'
port = int(args.PORT or 5013)

if args.LIBC:
    if args.LOCAL:
        libc = ELF(b'/usr/lib/x86_64-linux-gnu/libc.so.6')
    else:
        libc = ELF(b'libc.so (1).6_1ec728d58f7fc0d302119e9bb53050f8')

if exe.bits == 32:
    lindbg = "linux_server"
else:
    lindbg = "linux_server64"
    
def start_local(argv=[], *a, **kw):
    '''Execute the target binary locally'''
    if args.GDB:
        return gdb.debug([exe.path] + argv, gdbscript=gdbscript, *a, **kw)
    elif args.EDB:
        return process(['edb','--run',exe.path]+argv,*a,**kw)
    elif args.IDA:
        return process([lindbg],*a,**kw)
    else:
        return process([exe.path] + argv, *a, **kw)

def start_remote(argv=[], *a, **kw):
    '''Connect to the process on the remote host'''
    io = connect(host, port)
    if args.GDB:
        gdb.attach(io, gdbscript=gdbscript)
    return io

def start(argv=[], *a, **kw):
    '''Start the exploit against the target.'''
    if args.LOCAL:
        return start_local(argv, *a, **kw)
    else:
        return start_remote(argv, *a, **kw)

# Specify your GDB script here for debugging
# GDB will be launched if the exploit is run via e.g.
# ./exploit.py GDB
gdbscript = '''
continue
'''.format(**locals())

#===========================================================
#                    EXPLOIT GOES HERE
#===========================================================


io = start()
try:
    print(io.recvuntil("Guess a 64-bit number:"))
    result = subprocess.run(
        ['./guess_number'],  # путь к бинарнику и аргументы
        capture_output=True,    # захватываем stdout и stderr
        text=True,              # возвращаем строки вместо bytes
        check=True              # бросаем исключение при ненулевом коде возврата
    )
    stdout_output = result.stdout
    print(stdout_output)
    io.sendline(stdout_output)
    recv_str1=io.recv(1024)
    recv_str2=io.recv(1024)
    print(recv_str1)
    print(recv_str2)
    with open('file.txt', 'a') as file:
        file.write(recv_str1.decode())
        file.write(recv_str2.decode())
finally:
    pass
