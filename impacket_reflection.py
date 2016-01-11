# Our imports
from impacket.smbconnection import *
from impacket.ntlm import *
from impacket.spnego import SPNEGO_NegTokenResp, SPNEGO_NegTokenInit, TypesMech

# smb packet struct
sessionSetup = SMB2SessionSetup()
sessionSetup['Flags'] = 0
blob = SPNEGO_NegTokenInit()
blob['MechTypes'] = [TypesMech['NTLMSSP - Microsoft NTLM Security Support Provider']]

# build our own hash1
hash1 = getNTLMSSPType1(signingRequired = True)
hash1.dump()
# string to hash1 structure for convenience
h1 = NTLMAuthNegotiate()
h1.fromString(str(hash1))
# test to see if our hash contains a flag
# it will return True if it contains Flase if it does not
False if (h1['flags'] & NTLMSSP_SIGN) == 0 else True
# So lets remove the signing and sealing flags
h1['flags'] &= 0xffffffff ^ NTLMSSP_SIGN
# If we wanted to add it back in we could do
h1['flags'] |= NTLMSSP_SIGN


# formatting our hash1 structure:
blob['MechToken'] = str(hash1)
sessionSetup['SecurityBufferLength'] = len(blob)
sessionSetup['Buffer']  = blob.getData()

# initiate our connection
con = SMBConnection('127.0.0.1', '127.0.0.1', preferredDialect=SMB2_DIALECT_002) 
low_level = con.getSMBServer()

# packet structure
packet = low_level.SMB_PACKET()
packet['Command'] = SMB2_SESSION_SETUP
packet['Data'] = sessionSetup

# dump it
packet.dump()

# send it get the ID
pid = low_level.sendSMB(packet)

#recv the hash2
resp = low_level.recvSMB(pid)

# we HAVE to set our session ID for later use, we can do that like so.
low_level._Session['SessionID'] = resp['SessionID']

# hash2 into a structure for later use
sessionSetupResponse = SMB2SessionSetup_Response(resp['Data'])
respToken = SPNEGO_NegTokenResp(sessionSetupResponse['Buffer'])
hash2 = NTLMAuthChallenge()
hash2.fromString(respToken['ResponseToken'])

# now if we wanted to access the 8byte challenge we could do this ;)
hash2['challenge']

# also note that dump works as always
hash2.dump()


# the actual structure for formating the hash 3 that impacket uses is this NTLMAuthChallengeResponse
hash3, seskey = getNTLMSSPType3(hash1, str(hash2), 'admin_account', 'simpleadmin', 'WORKGROUP')

# If you want to use a nthash
#from binascii import a2b_hex
#nhash = a2b_hex('5a0d0336d04ae1031f2d36bd5bd0a759')
#hash3, seskey = getNTLMSSPType3(hash1, str(hash2), 'admin_account', '', 'WORKGROUP', nthash=nhash)


# set the Session key
low_level. _Session['SessionKey']  = seskey

# start struct like before
respToken = SPNEGO_NegTokenResp()
respToken['ResponseToken'] = str(hash3)

# finish smb structure
sessionSetup = SMB2SessionSetup()
sessionSetup['Flags'] = 0
sessionSetup['SecurityBufferLength'] = len(respToken)
sessionSetup['Buffer'] = respToken.getData()

# packet structure
packet = low_level.SMB_PACKET()
packet['Command'] = SMB2_SESSION_SETUP
packet['Data'] = sessionSetup

# finally send the packet
pid = low_level.sendSMB(packet)
resp = low_level.recvSMB(pid)

# sweet finished
shares = con.listShares()
for n in shares:
    n.dump()
