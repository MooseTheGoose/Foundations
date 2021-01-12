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
            using (StreamReader rdr = File.OpenText("..\\..\\testparseexpr.fnd"))
            {
                string fcontents = rdr.ReadToEnd().Replace("\r\n", "\n");
                Console.WriteLine("Expressions to parse:");
                Console.WriteLine(fcontents);
                Lexer lxr = new Lexer(fcontents);
                Parser prsr = new Parser(lxr.tokens.ToArray());

                while (prsr.PeekToken().type != TokenType.STREAM_TERM)
                {
                    while ((uint)prsr.PeekToken().type >= Token.StatementTermBase 
                        && (uint)prsr.PeekToken().type <= Token.StatementTermBase + Token.CategoryLimit
                        && prsr.PeekToken().type != TokenType.STREAM_TERM)
                    {
                        prsr.EatToken();
                    }

                    if (prsr.PeekToken().type != TokenType.STREAM_TERM)
                    {
                        Console.WriteLine("Parsing next expression...\n");
                        DerivationTree expr = prsr.ParseExpression();
                        if (expr.children != null)
                        {
                            Console.WriteLine("Final expression: " + expr.children.PrintTree() + "\n");
                        }
                        else
                        {
                            Console.WriteLine("ERROR: expression parsing was unsuccessful. Exiting with code -1...");
                            Environment.Exit(-1);
                        }
                    }
                }
            }
            Console.WriteLine("Done! Press Any Key to Continue...");
            Console.ReadKey();
        }
    }
}
