using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Foundations
{
    class Foundations
    {
        public static void Main(string[] args)
        {
            using (StreamReader rdr = File.OpenText("..\\..\\testlexer.fnd"))
            {
                string fcontents = rdr.ReadToEnd().Replace("\r\n", "\n");
                Lexer lxr = new Lexer(fcontents);
                foreach (Token t in lxr.tokens)
                {
                    Console.WriteLine(t.type);
                    Console.WriteLine(t.lino);
                    Console.WriteLine(t.chno);
                    Console.WriteLine(t.data ?? "null");
                    Console.WriteLine();
                }
            }

            Console.ReadKey();
        }
    }
}
