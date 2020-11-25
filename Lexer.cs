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
        WARNING = 4,
        IDENTIFIER = 5,
        STRING = 6,

        OP_DEFN = 0x100,
        OP_MINUS = 0x101,
        OP_PLUS = 0x102,
        OP_ASGN = 0x103,
        OP_MULT = 0x104,
        OP_DIV = 0x105,
        OP_MOD = 0x106,
        OP_ADDR = 0x107,

        KW_UINT = 0x200,
        KW_U64 =  0x201,
        KW_U32 =  0x202,
        KW_U16 =  0x203,
        KW_U8 =   0x204,
        KW_INT =  0x205,
        KW_S64 =  0x206,
        KW_S32 =  0x207,
        KW_S16 =  0x208,
        KW_S8 =   0x209,

        NUM_UINT64 = 0x300,
        NUM_FLOAT32 = 0x301,
        NUM_FLOAT64 = 0x302
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
            TokenType.OP_ADDR, TokenType.OP_DIV, TokenType.OP_MULT, TokenType.OP_MOD,
            TokenType.OP_DEFN, TokenType.OP_MINUS, TokenType.OP_PLUS, TokenType.OP_ASGN
        };

        public string[] DelimiterStrings = {
            "!", "/", "*", "%",
            ":", "-", "+", "="
        };

        public TokenType[] KeywordTypes = {
            TokenType.KW_UINT, TokenType.KW_U64, TokenType.KW_U32, TokenType.KW_U16, TokenType.KW_U8,
            TokenType.KW_INT, TokenType.KW_S64, TokenType.KW_S32, TokenType.KW_S16, TokenType.KW_S8
        };

        public string[] KeywordStrings = {
            "uint", "u64", "u32", "u16", "u8",
            "int", "s64", "s32", "s16", "s8"
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

        public static int HexDigitToValue(char ch)
        {
            int value = -1;
            if (ch <= '9')
            {
                value = (int)ch - '0';
            }
            else if(ch >= 'A')
            {
                value = (int)Char.ToUpper(ch) - 'A' + 10;
            }
            return value;
        }

        public static bool IsXDigit(char ch)
        {
            int val = Lexer.HexDigitToValue(ch);
            return val >= 0 && val < 16;
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

            Token newToken = new Token(TokenType.IDENTIFIER, this.currLino, this.currChno, null);

            while (currChar == '_' || Char.IsLetterOrDigit(currChar))
            {
                build.Append(currChar);
                this.EatChar();
                currChar = this.PeekChar();
            }


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

        public char LexNumber()
        {
            StringBuilder num = new StringBuilder();
            char currChar = this.PeekChar();
            int radix = 10;
            int digit;

            Token newToken = new Token(TokenType.NUM_UINT64, this.currLino, this.currChno, null);

            if (currChar == '0')
            {
                this.EatChar();
                currChar = this.PeekChar();

                if (Char.ToLower(currChar) == 'x')
                {
                    radix = 16;
                    this.EatChar();
                    currChar = this.PeekChar();
                }
                else if (Char.ToLower(currChar) == 'b')
                {
                    radix = 2;
                    this.EatChar();
                    currChar = this.PeekChar();
                }
                else if (Char.IsDigit(currChar) || currChar == '_')
                {
                    radix = 8;
                    if (currChar > '7' && currChar != '_')
                    {
                        Token errToken = new Token(TokenType.ERROR, this.currLino, this.currChno, "Expected octal digit here");
                        this.tokens.Add(errToken);
                        while (currChar == '_' || Char.IsLetterOrDigit(currChar))
                        {
                            this.EatChar();
                            currChar = this.PeekChar();
                        }
                        return currChar;
                    }
                }
                else if(Char.IsLetterOrDigit(currChar))
                {
                    Token errToken = new Token(TokenType.ERROR, this.currLino, this.currChno, "Bad base identifier in numeric literal");
                    this.tokens.Add(errToken);
                    while (currChar == '_' || Char.IsLetterOrDigit(currChar))
                    {
                        this.EatChar();
                        currChar = this.PeekChar();
                    }
                    return currChar;
                }
            }

            while (currChar == '_')
            {
                this.EatChar();
                currChar = this.PeekChar();
            }

            digit = Lexer.HexDigitToValue(currChar);
            while (digit >= 0 && digit < radix)
            {
                while (currChar == '_')
                {
                    this.EatChar();
                    currChar = this.PeekChar();
                }

                digit = HexDigitToValue(currChar);
                if (digit >= 0 && digit < radix)
                {
                    num.Append(currChar);
                    this.EatChar();
                    currChar = this.PeekChar();
                }
            }

            if ((currChar == 'e' || currChar == '.') && radix == 10)
            {
                double tryFloat64;
                float tryFloat32;

                if (currChar == '.')
                {
                    this.EatChar();
                    currChar = this.PeekChar();
                    num.Append('.');
                }

                while (currChar == '_')
                {
                    this.EatChar();
                    currChar = this.PeekChar();
                }
                while (Char.IsDigit(currChar))
                {
                    num.Append(currChar);
                    this.EatChar();
                    currChar = this.PeekChar();
                    while (currChar == '_')
                    {
                        this.EatChar();
                        currChar = this.PeekChar();
                    }
                }

                if (!Char.IsDigit(num[num.Length - 1]) && currChar != 'e')
                {
                    num.Append('0');
                }

                if (currChar == 'e')
                {
                    num.Append(currChar);
                    this.EatChar();
                    currChar = this.PeekChar();
                    if (currChar == '-' || currChar == '+')
                    {
                        num.Append(currChar);
                        this.EatChar();
                        currChar = this.PeekChar();
                    }

                    while (currChar == '_')
                    {
                        this.EatChar();
                        currChar = this.PeekChar();
                    }
                    while (Char.IsDigit(currChar))
                    {
                        num.Append(currChar);
                        this.EatChar();
                        currChar = this.PeekChar();
                        while (currChar == '_')
                        {
                            this.EatChar();
                            currChar = this.PeekChar();
                        }
                    }

                    if (!Char.IsDigit(num[num.Length - 1]))
                    {
                        Token errToken = new Token(TokenType.ERROR, this.currLino, this.currChno, "Expected decimal integer after exponent specifier");
                        this.tokens.Add(errToken);
                        while (currChar == '_' || Char.IsLetterOrDigit(currChar))
                        {
                            this.EatChar();
                            currChar = this.PeekChar();
                        }
                        return currChar;
                    }
                }

                try
                {
                    tryFloat64 = Convert.ToDouble(num.ToString());
                }
                catch (OverflowException)
                {
                    newToken.type = TokenType.WARNING;
                    newToken.data = "Floating point value expressed here overflows";
                    this.tokens.Add(newToken);
                    while (currChar == '_' || Char.IsLetterOrDigit(currChar))
                    {
                        this.EatChar();
                        currChar = this.PeekChar();
                    }
                    return currChar;
                }

                tryFloat32 = (float)tryFloat64;
                if (tryFloat32 == tryFloat64)
                {
                    newToken.type = TokenType.NUM_FLOAT32;
                    newToken.data = tryFloat32;
                }
                else
                {
                    newToken.type = TokenType.NUM_FLOAT64;
                    newToken.data = tryFloat64;
                }
                this.tokens.Add(newToken);
            }
            else
            {
                if (num.Length == 0)
                {
                    Token errToken = new Token(TokenType.ERROR, this.currLino, this.currChno, "Number has no digits after base");
                    this.tokens.Add(errToken);
                    while (currChar == '_' || Char.IsLetterOrDigit(currChar))
                    {
                        this.EatChar();
                        currChar = this.PeekChar();
                    }
                    return currChar;
                }

                try
                {
                    ulong u64val = Convert.ToUInt64(num.ToString(), radix);
                    newToken.data = u64val;
                }
                catch (OverflowException)
                {
                    newToken.type = TokenType.WARNING;
                    newToken.data = "Integer value expressed overflows";
                }

                this.tokens.Add(newToken);
            }

            return currChar;
        }

        public char LexString()
        {
            char quote = this.EatChar();
            char currChar = this.PeekChar();
            StringBuilder builder = new StringBuilder();
            Token newToken = new Token(TokenType.STRING, this.currLino, this.currChno, null);

            while (currChar != quote)
            {
                if (currChar == '\\')
                {
                    ProcessEscape(builder);
                    currChar = this.PeekChar();
                }
                else if (currChar == '\n' || currChar == '\0')
                {
                    string message = "Unexpected line separator in string. Did you mean to put a \\ here?";
                    if (currChar == '\0')
                    {
                        message = "String is incomplete on EOF";
                    }
                    this.tokens.Add(new Token(TokenType.ERROR, this.currLino, this.currChno, message));
                    while (currChar != '\0' && currChar != quote)
                    {
                        this.EatChar();
                        currChar = this.PeekChar();
                    }
                    this.EatChar();
                    currChar = this.PeekChar();
                    return currChar;
                }
                else 
                {
                    builder.Append(currChar);
                    this.EatChar();
                    currChar = this.PeekChar();
                }
            }

            this.EatChar();
            currChar = this.PeekChar();
            newToken.data = builder.ToString();
            this.tokens.Add(newToken);
            return currChar;
        }

        public void ProcessEscape(StringBuilder str)
        {
            this.EatChar();
            char currChar = this.PeekChar();
            this.EatChar();

            bool pushChar = true;

            switch (currChar)
            {
                case 'n':
                    currChar = '\n';
                    break;
                case 'r':
                    currChar = '\r';
                    break;
                case '0':
                    currChar = '\0';
                    break;
                case 'x':
                    char hexDigit1 = this.EatChar();
                    char hexDigit2 = this.EatChar();
                    if (!Lexer.IsXDigit(hexDigit1) || !Lexer.IsXDigit(hexDigit2))
                    {
                        Token errToken = new Token(TokenType.ERROR, this.currLino, this.currChno, "Expected 2 hex digits for \\x escape");
                        this.tokens.Add(errToken);
                    }
                    currChar = (char)((byte)(Lexer.HexDigitToValue(hexDigit1) << 4 | Lexer.HexDigitToValue(hexDigit2)));
                    break;
                case '\n':
                    pushChar = false;
                    break;
                default:
                    break;
            }

            if (pushChar)
            {
                str.Append(currChar);
            }
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
                else if (currChar == '\'' || currChar == '"')
                {
                    currChar = this.LexString();
                }
                else
                {
                    Token newToken = new Token(TokenType.ERROR, this.currLino, this.currChno, "Unexpected Character");
                    this.EatChar();
                    currChar = this.PeekChar();
                    this.tokens.Add(newToken);
                }
            }

            this.tokens.Add(new Token(TokenType.TERM, this.currLino, this.currChno, null));
        }
    }
}
