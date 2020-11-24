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
            using (StreamReader rdr = File.OpenText("..\\..\\Foundations.cs"))
            {
                string fcontents = rdr.ReadToEnd();
                Console.WriteLine("Printing Myself");
                Console.WriteLine("---------------");
                Console.WriteLine(fcontents);
                Console.ReadKey();
            }
        }
    }
}
