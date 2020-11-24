using System;
using System.Numerics;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Foundations
{
    enum TokenType
    {
        TERM = 0,
        NEWLINE = 1,
        STATEMENT_TERM = 2,
        ERROR = 3,
        IDENTIFIER = 4,
        STRING = 5,

        OP_COLON = 0x1000,
        OP_MINUS = 0x1001,
        OP_PLUS = 0x1002,
        OP_ASGN = 0x1003,

        KW_INT = 0x2000,

        NUM_INT = 0x3000,
        NUM_FLOAT = 0x3001
    }

    struct Token
    {
        public TokenType type;
        public int lino;
        public int chno;
        public object data;

        public Token(TokenType t, int ln, int cn, object obj)
        {
            this.type = t;
            this.lino = ln;
            this.chno = cn;
            this.data = obj;
        }
    }

    class Lexer
    {
        public int currLino;
        public int currChno;
        public int srcIndex;
        public string src;

        public List<Token> tokens;
        public const char LineSep = '\n';
        public const char StatementTerm = ';';

        public TokenType[] DelimiterTypes = {
            TokenType.OP_COLON, TokenType.OP_MINUS, TokenType.OP_PLUS, TokenType.OP_ASGN
        };

        public string[] DelimiterStrings = {
            ":", "-", "+", "="
        };

        public TokenType[] KeywordTypes = {
            TokenType.KW_INT
        };

        public string[] KeywordStrings = {
            "int"
        };

        public Lexer(string source)
        {
            this.currLino = 1;
            this.currChno = 1;
            this.srcIndex = 0;
            this.src = source;

            this.tokens = new List<Token>();

            this.Lex();
        }

        public char EatChar()
        {
            char ch = '\0';

            if (this.srcIndex < this.src.Length)
            {
                ch = this.src[this.srcIndex];
                this.srcIndex += 1;
                this.currChno += 1;

                if (ch == LineSep)
                {
                    this.currLino += 1;
                    this.currChno = 1;
                }
            }

            return ch;
        }

        public char PeekChar()
        {
            char ch = '\0';

            if (this.srcIndex < this.src.Length)
            {
                ch = this.src[this.srcIndex];
            }

            return ch;
        }

        public bool StartsWith(string prefix)
        {
            return String.Compare(this.src, this.srcIndex, prefix, 0, prefix.Length) == 0;
        }

        /*
         * NOTE: It's important to order the prefixes such that
         *       proper substrings have higher indicies.
         */
        public int StartsWithIndex(string[] prefixes)
        {
            int index = -1;

            for (int i = 0; i < prefixes.Length && index < 0; i++)
            {
                if (this.StartsWith(prefixes[i]))
                {
                    index = i;
                }
            }

            return index;
        }

        public static int StringStartsWithIndex(string startsWith, string[] prefixes)
        {
            int index = -1;

            for (int i = 0; i < prefixes.Length && index < 0; i++)
            {
                if (startsWith.StartsWith(prefixes[i]))
                {
                    index = i;
                }
            }

            return index;
        }

        public char LexLineSep()
        {
            Token newToken = new Token(TokenType.NEWLINE, this.currLino, this.currChno, null);
            this.tokens.Add(newToken);
            this.EatChar();
            return this.PeekChar();
        }

        public char LexStatementTerm()
        {
            Token newToken = new Token(TokenType.STATEMENT_TERM, this.currLino, this.currChno, null);
            this.tokens.Add(newToken);
            this.EatChar();
            return this.PeekChar();
        }

        public char LexDelimiter(int index)
        {
            Token newToken = new Token(this.DelimiterTypes[index], this.currLino, this.currChno, null);
            this.tokens.Add(newToken);

            for (int i = 0; i < this.DelimiterStrings[index].Length; i++)
            {
                this.EatChar();
            }

            return this.PeekChar();
        }

        public char LexIdentifierOrKeyword()
        {
            char currChar = this.PeekChar();
            int kwIndex;
            StringBuilder build = new StringBuilder();

            while (currChar == '_' || Char.IsLetterOrDigit(currChar))
            {
                build.Append(currChar);
                this.EatChar();
                currChar = this.PeekChar();
            }

            Token newToken = new Token(TokenType.IDENTIFIER, this.currLino, this.currChno, null);
            string finalStr = build.ToString();
            if ((kwIndex = Lexer.StringStartsWithIndex(finalStr, this.KeywordStrings)) >= 0)
            {
                newToken.type = this.KeywordTypes[kwIndex];
            }
            else
            {
                newToken.data = finalStr;
            }
            this.tokens.Add(newToken);

            return this.PeekChar();
        }

        /*
         * HAVEN'T STARTED YET!
         */
        public char LexNumber()
        {
            this.EatChar();
            return this.PeekChar();
        }

        public void Lex()
        {
            char currChar = this.PeekChar();
            int index;

            while (currChar != '\0')
            {
                while (Char.IsWhiteSpace(currChar) && currChar != LineSep)
                {
                    this.EatChar();
                    currChar = this.PeekChar();
                }
                if (currChar == LineSep)
                {
                    currChar = this.LexLineSep();
                }
                else if (currChar == StatementTerm)
                {
                    currChar = this.LexStatementTerm();
                }
                else if ((index = this.StartsWithIndex(this.DelimiterStrings)) >= 0)
                {
                    currChar = this.LexDelimiter(index);
                }
                else if (currChar == '_' || Char.IsLetter(currChar))
                {
                    currChar = this.LexIdentifierOrKeyword();
                }
                else if (Char.IsDigit(currChar))
                {
                    currChar = this.LexNumber();
                }
                else
                {
                    Token newToken = new Token(TokenType.ERROR, this.currLino, this.currChno, null);
                    this.EatChar();
                    currChar = this.PeekChar();
                    this.tokens.Add(newToken);
                }
            }

            this.tokens.Add(new Token(TokenType.TERM, this.currLino, this.currChno, null));
        }
    }
}
