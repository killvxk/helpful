using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.RegularExpressions;


/*
string pi = "\u03a0";
byte[] ascii = System.Text.Encoding.ASCII.GetBytes(pi);
byte[] utf8 = System.Text.Encoding.UTF8.GetBytes(pi);
 */

/*
b.toString(2); //binary
b.toString(8); //octal
b.toString(10); //decimal
b.toString(16); //hexadecimal
 */
/* string literal
string[] hexValues = Regex.Split(new_in, "(?<=\\G..)(?!$)");
var characters = hexValues.Select(hex => (char)Convert.ToByte(hex, 16));
Console.Write(string.Join(string.Empty, characters));
*/
namespace teach_hex
{
    class Program
    {
        // haha ya pretty nifty!
        private static bool ntlm_check(string input)
        {
            try
            {
                Match m = Regex.Match(input, @"4E-54-4C-4D-53-53-50.{4}(?<msg>\d{2})[-0-9a-zA-Z]*");
                if (m.Value.Length != 0)
                {
                    Console.WriteLine("NTLM Search Value  = " + m.Value);
                    String[] arr = m.Value.Split('-');
                    byte[] array = new byte[arr.Length];
                    for (int i = 0; i < arr.Length; i++) array[i] = Convert.ToByte(arr[i], 16);
                    if (array[8] == 1)
                    {
                        Console.WriteLine("You have a NTLM message 1!!");
                    }
                    else if (array[8] == 2)
                    {
                        Console.WriteLine("You have a NTLM message 2!!");
                    }
                    else if (array[8] == 3)
                    {
                        Console.WriteLine("You have a NTLM message 3!!");
                    }
                    return true;
               }
                return false;
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                return false;
            }
        }

        // print binary
        private static void print_bin(string input)
        {
            string newinput = null;
            try
            {
                newinput = input.Replace("binary: ", "");
            }
            catch (Exception)
            {
                Console.WriteLine("Call binary: ");
            }
            Console.ForegroundColor = ConsoleColor.Red;
            char[] input_chr_array = newinput.ToCharArray();
            Console.Write("Bin: ");
            foreach (char letter in input_chr_array)
            {
                int value = Convert.ToInt32(letter);
                string binOutput = Convert.ToString(value, 2);
                Console.Write(" {0} ", binOutput);
            }
            Console.Write("\n");
            Console.ResetColor();
        }

        // Printing to Int
        static void print_int(string input)
        {
            // so you can see it clearly
            Console.ForegroundColor = ConsoleColor.Red;
            char[] input_chr_array = input.ToCharArray();
            Console.Write("Int : "); // so it prints nicely!
            foreach (char letter in input_chr_array)
            {
                // The int value of the char
                int value = Convert.ToInt32(letter);
                // Convert the decimal value to a hex in string form
                string intOutput = value.ToString("D3"); // to 3
                Console.Write(" {0} ", intOutput);
            }
            Console.Write("\n");
            Console.ResetColor();
        }

        // Printing to hex
        static void print_hex(string input)
        {
            // so you can see it clearly
            Console.ForegroundColor = ConsoleColor.Blue;
            char[] input_chr_array = input.ToCharArray();
            Console.Write("Hex : "); // so it prints nicely!
            foreach (char letter in input_chr_array)
            {
                // The int value of the char
                int value = Convert.ToInt32(letter);
                // Convert the decimal value to a hex in string form.
                string hexOutput = String.Format("{0:X}", value);
                Console.Write("\\x{0} ", hexOutput);
            }
            Console.Write("\n");
            Console.ResetColor();
        }

        // Printing the Orignal ascii
        static void print_orig(string input)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            char[] input_chr_array = input.ToCharArray();
            Console.Write("Asci: ");
            foreach (char letter in input_chr_array)
            {
                Console.Write("  {0}  ", letter);
            }
            Console.Write("\n");
            Console.ResetColor();
        }

        // bullshit ms crap really
        static void print_ms_hex_to_chr(string input)
        {
            String newinput = null;
            try
            {
                newinput = input.Replace("ms: ", "");
            }
            catch (Exception)
            {
                Console.WriteLine("Call binary: ");
            }
            //Call our normal print
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Write("Result: ");
            String new_in = newinput.Replace("-", "");
            string[] hex_value = Regex.Split(new_in, "(?<=\\G..)(?!$)");
            foreach (string letter in hex_value)
            {
                Console.Write(" {0}", (char)Convert.ToByte(letter, 16));
            }
            Console.Write("\n");
            Console.ResetColor();
        }

        // preint ms crap to python, or any other lang.
        static void print_ms_hex_to_realhex(string input)
        {
            String newinput = null;
            try
            {
                newinput = input.Replace("ms: ", "");
            }
            catch (Exception)
            {
                Console.WriteLine("Call binary: ");
            }
            Console.ForegroundColor = ConsoleColor.White;
            Console.Write("RealHex = \"");
            String new_in = newinput.Replace("-", "");
            string[] hex_value = Regex.Split(new_in, "(?<=\\G..)(?!$)");
            foreach (string letter in hex_value)
            {
                Console.Write("\\x{0}", letter);
            }
            Console.WriteLine("\"");
            Console.Write("\n");
            Console.ResetColor();
        }


        // Print the converted hex to char
        static void print_hex_to_chr(string input)
        {
            //Call our normal print
            Console.ForegroundColor = ConsoleColor.Green;
            Console.Write("Asci: ");
            String new_in = input.Replace("\\x", "");
            string[] hex_value = Regex.Split(new_in, "(?<=\\G..)(?!$)");
            foreach (string letter in hex_value)
            {
                Console.Write("  {0}  ", (char)Convert.ToByte(letter, 16));
            }
            Console.Write("\n");
            Console.ResetColor();
        }

        // a checking routine
        static bool hex_binary_check(string input)
        {
            if (input.Trim().StartsWith("\\x"))
            {
                print_hex_to_chr(input);
                return true;
            }
            else if (input.Trim().StartsWith("binary"))
            {
                print_bin(input);
                return true;
            }
            else if (input.Trim().StartsWith("ms"))
            {
                print_ms_hex_to_chr(input);
                print_ms_hex_to_realhex(input);
                return true;
            }
            else if (ntlm_check(input))
            {
                return true; // it already prited etc..
            }
            else
            {
                return false;
            }
        }

        static void Main()
        {
            while (true)
            {
                Console.Write("Input: "); String input = Console.ReadLine();
                try
                {

                    if (hex_binary_check(input)) // returns true/false
                    {
                        //pass its empty!
                    }
                    else
                    {
                        print_int(input);
                        print_hex(input);
                        print_orig(input);
                    }
                }
                catch (Exception)
                {
                    Console.ResetColor();
                    Console.WriteLine("\nWhat are you doing dave?");
                    Console.WriteLine("Im sorry dave but I can't let you do that.\n");
                }
            }
        }
    }
}
