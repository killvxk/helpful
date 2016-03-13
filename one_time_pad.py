import sys
from Crypto import Random
# we need truly random bytes this is close, just dont concentrate on it!1! ;)

psize = 1024
count = 100

def write_pad(file):
    print("Generating %s pads with padsize starting at %s" % (count, psize))
    rndfile = Random.new()
    out_bytes = ""
    for i in range(count):
        out_bytes += rndfile.read(psize * i)
    write_file(file, out_bytes)
    print("Wrote %s bytes to file %s" % (len(out_bytes), file))

def write_file(file, bytes):
    o = open(file, "wb")
    o.write(bytes)
    o.close

def open_file(file):
    o = open(file, "rb")
    ret = o.read()
    o.close()
    return ret

def encrypt_decrypt_file(infile, padfile, outfile, pad_number=''):
    fbytes = open_file(infile)
    pbytes = open_file(padfile)
    length_needed = (len(fbytes) / psize) + 1
    if pad_number and length_needed < pad_number: length_needed = pad_number
    else: print "User defined pad to small using: %s" % length_needed
    pad_tmp = 0
    for i in range(length_needed):
        pad_tmp = (pad_tmp + (psize * i))
    our_padding = pbytes[pad_tmp-(psize * length_needed):pad_tmp]
    print("Using section: [%s:%s], pad number %s DONT USE IT AGAIN!" % \
        (pad_tmp-(psize * length_needed), pad_tmp, length_needed))
    e_bytes = ""
    for i in range(len(fbytes)):
        e_bytes += chr(ord(fbytes[i]) ^ ord(our_padding[i]))
    write_file(outfile, e_bytes)

def omg_halps():
    print("python %s <genpad> <outfile> [pad_count] [pad_size]" % sys.argv[0])
    print("python %s <encrypt> <infile> <padfile> <outfile> [pad_number]" % sys.argv[0])
    print("python %s <decrypt> <infile> <padfile> <outfile> [pad_number]" % sys.argv[0])
    # "python %s <encrypt> <infile> <padfile> <outfile> [pad_number] [pad_count] [pad_size]"
    # "python %s <decrypt> <infile> <padfile> <outfile> [pad_number] [pad_count] [pad_size]"

# next time use argparse!
if __name__ == "__main__":
    if len(sys.argv) < 3:
        omg_halps()
        sys.exit(0)
    
    option = sys.argv[1]
    try:
        if option == "genpad":
            if len(sys.argv) == 4:
                count = int(sys.argv[3])
            if len(sys.argv) == 5:
                count = int(sys.argv[3])
                psize = int(sys.argv[4])
            write_pad(sys.argv[2])
        elif option == "encrypt":
            pad_number = ''
            if len(sys.argv) == 6:
                pad_number = int(sys.argv[5])
            if len(sys.argv) == 7:
                psize = int(sys.argv[6])
            if len(sys.argv) == 8:
                psize = int(sys.argv[7])
                count = int(sys.argv[8])
            print("Encrypting %s using padfile %s and writing to file %s" % (sys.argv[2], sys.argv[3], sys.argv[4]))
            encrypt_decrypt_file(sys.argv[2], sys.argv[3], sys.argv[4], pad_number)
        elif option == "decrypt":
            pad_number = ''
            if len(sys.argv) == 6:
                pad_number = int(sys.argv[5])
            if len(sys.argv) == 7:
                psize = int(sys.argv[6])
            if len(sys.argv) == 8:
                psize = int(sys.argv[7])
                count = int(sys.argv[8])
            print("Decrypting %s using padfile %s and writing to file %s" % (sys.argv[2], sys.argv[3], sys.argv[4]))
            encrypt_decrypt_file(sys.argv[2], sys.argv[3], sys.argv[4], pad_number)
        else:
            omg_halps()
    except Exception as e:
        print e
        print("Im sorry but I can't allow you to do that!")
        omg_halps()
