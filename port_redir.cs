using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
// cd C:\Windows\Microsoft.NET\Framework\v4*
// csc.exe  /out:"C:\Utils\port_redir.exe" /platform:x86 "C:\Utils\port_redir.cs"
class Simple_Program
{
    // The socket relay Receive bytes, Send them back
    private static void Sock_Relay(Socket sock1, Socket sock2)
    {
        Byte[] bytes = new Byte[1024];
        while (true)
        {
            try
            {
                int bytesrec = sock1.Receive(bytes);
                sock2.Send(bytes, bytesrec, 0);
            }
            catch (Exception e) { Console.WriteLine("{0}", e); Environment.Exit(0);}
        }
    }
    // How we make the outbound Connection.
    static Socket Connect_Call(String con_ip, int con_p)
    {
        IPEndPoint ip_con = new IPEndPoint(IPAddress.Parse(con_ip), con_p);
        Socket s = new Socket(ip_con.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
        s.Connect(ip_con);
        return s;
    }
    // main.. how we start it all up
    static int Main(string[] args)
    {
        // args checker
        if (args.Length != 4)
        {
            System.Console.WriteLine("port_redir.exe <listenIP> <listenPort> <connectIP> <connectPort>");
            return 1;
        }
        // Our socket bind "function" we could just as easily replace this with
        // A Connect_Call and make it a true gender-bender
        IPEndPoint tmpsrv = new IPEndPoint(IPAddress.Parse(args[0]), Int32.Parse(args[1]));
        Socket ipsrv = new Socket(tmpsrv.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
        ipsrv.Bind(tmpsrv);
        ipsrv.Listen(10);
        
        // loop it so we can continue to accept
        while (true)
        {
            try
            {
                // block till something connects to us
                Socket srv = ipsrv.Accept();
                // Once it does connect to the outbound ip:port
                Socket con = Connect_Call(args[2], Int32.Parse(args[3]));
                // Read and write back and forth to our sockets.
                Thread SrvToCon = new Thread(() => Sock_Relay(srv, con));
                SrvToCon.Start();
                Thread ConToSrv = new Thread(() => Sock_Relay(con, srv));
                ConToSrv.Start();
            }
            catch (Exception e) { Console.WriteLine("{0}", e); }
        }
    }
}
