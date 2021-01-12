using System;
using System.Numerics;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Foundations
{
    enum TokenType
    {
        ERROR = 3,
        WARNING = 4,

        OP_DEFN = 0x10000,
        OP_MINUS = 0x10001,
        OP_PLUS = 0x10002,
        OP_ASGN = 0x10003,
        OP_MULT = 0x10004,
        OP_DIV = 0x10005,
        OP_MOD = 0x10006,
        OP_ADDR = 0x10007,
        OP_BWAND = 0x10008,
        OP_BWXOR = 0x10009,
        OP_BWOR = 0x1000A,
        OP_NEQ = 0x1000E,
        OP_EQ = 0x1000F,
        OP_GT = 0x10010,
        OP_LT = 0x10011,
        OP_GE = 0x10012,
        OP_LE = 0x10013,
        OP_DOT = 0x10014,
        OP_ARROW = 0x10015,
        OP_LSHIFT = 0x10016,
        OP_RSHIFT = 0x10017,
        OP_PLUS_ASGN = 0x10018,
        OP_MINUS_ASGN = 0x10019,
        OP_MULT_ASGN = 0x1001A,
        OP_DIV_ASGN = 0x1001B,
        OP_MOD_ASGN = 0x1001C,
        OP_LSHIFT_ASGN = 0x1001D,
        OP_RSHIFT_ASGN = 0x1001E,
        OP_BWAND_ASGN = 0x1001F,
        OP_BWXOR_ASGN = 0x10020,
        OP_BWOR_ASGN = 0x10021,
        OP_POLYTYPE = 0x10022,
        OP_COMMA = 0x10023,
        OP_BWNOT = 0x10024,
        OP_DECIDES = 0x10026,
        OP_COALESCE = 0x10027,
        OP_DEREF = 0x10028,

        TYPE_UINT = 0x20000,
        TYPE_U64 =  0x20001,
        TYPE_U32 =  0x20002,
        TYPE_U16 =  0x20003,
        TYPE_U8 =   0x20004,
        TYPE_INT =  0x20005,
        TYPE_S64 =  0x20006,
        TYPE_S32 =  0x20007,
        TYPE_S16 =  0x20008,
        TYPE_S8 =   0x20009,

        VAL_UINT64 = 0x30000,
        VAL_FLOAT32 = 0x30001,
        VAL_FLOAT64 = 0x30002,
        VAL_STRING = 0x30003,
        VAL_IDENTIFIER = 0x30004,


        BRACKET_LPAREN = 0x40000,
        BRACKET_RPAREN = 0x40001,
        BRACKET_LCURLY = 0x40002,
        BRACKET_RCURLY = 0x40003,
        BRACKET_LSQUARE = 0x40004,
        BRACKET_RSQUARE = 0x40005,

        NEWLINE_TERM = 0x50000,
        STATEMENT_TERM = 0x50001,
        STREAM_TERM = 0x50002
    }

    struct Token
    {
        public TokenType type;
        public int lino;
        public int chno;
        public object data;

        public const uint CategoryLimit = 0xFFFF;
        public const uint UncategorizedBase = 0x00000;
        public const uint OperatorBase = 0x10000;
        public const uint TypeBase = 0x20000;
        public const uint ValueBase = 0x30000;
        public const uint BracketBase = 0x40000;
        public const uint StatementTermBase = 0x50000;

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
        public List<Danger> errs;
        public const char LineSep = '\n';
        public const char StatementTerm = ';';

        /*
         * Note: These delimiters must be ordered by descending length.
         */

        public static TokenType[] DelimiterTypes = {
            TokenType.OP_LSHIFT_ASGN, TokenType.OP_RSHIFT_ASGN,
            TokenType.OP_PLUS_ASGN, TokenType.OP_MINUS_ASGN,
            TokenType.OP_MULT_ASGN, TokenType.OP_DIV_ASGN, TokenType.OP_MOD_ASGN,
            TokenType.OP_BWAND_ASGN, TokenType.OP_BWXOR_ASGN, TokenType.OP_BWOR_ASGN,
            TokenType.OP_LSHIFT, TokenType.OP_RSHIFT,
            TokenType.OP_NEQ, TokenType.OP_EQ,
            TokenType.OP_LE, TokenType.OP_GE,
            TokenType.OP_COALESCE,
            TokenType.OP_LT, TokenType.OP_GT,
            TokenType.OP_DOT, TokenType.OP_COMMA, TokenType.OP_POLYTYPE, TokenType.OP_ADDR,
            TokenType.OP_BWNOT, TokenType.OP_BWAND, TokenType.OP_BWXOR, TokenType.OP_BWOR,
            TokenType.OP_DIV, TokenType.OP_MULT, TokenType.OP_MOD,
            TokenType.OP_DEFN, TokenType.OP_MINUS, TokenType.OP_PLUS, TokenType.OP_ASGN,
            TokenType.BRACKET_LSQUARE, TokenType.BRACKET_LPAREN, TokenType.BRACKET_LCURLY,
            TokenType.BRACKET_RSQUARE, TokenType.BRACKET_RPAREN, TokenType.BRACKET_RCURLY,
            TokenType.OP_DEREF
        };

        public static string[] DelimiterStrings = {
            "<<=", ">>=",
            "+=", "-=",
            "*=", "/=", "%=",
            "&=", "^=", "|=",
            "<<", ">>",
            "~=", "==",
            "<=", ">=",
            "??",
            "<", ">",
            ".", ",", "?", "!",
            "~", "&", "^", "|",
            "/", "*", "%",
            ":", "-", "+", "=",
            "[", "(", "{",
            "]", ")", "}",
            "#"
        };

        /*
         * Note: These keywords can be in any order, unlike delimiters.
         */

        public static TokenType[] KeywordTypes = {
            TokenType.OP_DECIDES,
            TokenType.TYPE_UINT, TokenType.TYPE_U64, TokenType.TYPE_U32, TokenType.TYPE_U16, TokenType.TYPE_U8,
            TokenType.TYPE_INT, TokenType.TYPE_S64, TokenType.TYPE_S32, TokenType.TYPE_S16, TokenType.TYPE_S8
        };

        public static string[] KeywordStrings = {
            "decides", 
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
            this.errs = new List<Danger>();

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
            Token newToken = new Token(TokenType.NEWLINE_TERM, this.currLino, this.currChno, null);
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
            Token newToken = new Token(Lexer.DelimiterTypes[index], this.currLino, this.currChno, null);
            this.tokens.Add(newToken);

            for (int i = 0; i < Lexer.DelimiterStrings[index].Length; i++)
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

            Token newToken = new Token(TokenType.VAL_IDENTIFIER, this.currLino, this.currChno, null);

            while (currChar == '_' || Char.IsLetterOrDigit(currChar))
            {
                build.Append(currChar);
                this.EatChar();
                currChar = this.PeekChar();
            }


            string finalStr = build.ToString();
            if ((kwIndex = Array.IndexOf(Lexer.KeywordStrings, finalStr)) >= 0)
            {
                newToken.type = Lexer.KeywordTypes[kwIndex];
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

            Token newToken = new Token(TokenType.VAL_UINT64, this.currLino, this.currChno, null);

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
                        Danger errToken = new Danger(this.currLino, this.currChno, Danger.ERROR_SEVERITY, Danger.EINTEGER_PARSE_NUM, "Expected octal digit here");
                        this.errs.Add(errToken);
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
                    Danger errToken = new Danger(this.currLino, this.currChno, Danger.ERROR_SEVERITY, Danger.EINTEGER_PARSE_NUM, "Bad base identifier in numeric literal");
                    this.errs.Add(errToken);
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

                    while (currChar == '_')
                    {
                        this.EatChar();
                        currChar = this.PeekChar();
                    }
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
                        Danger errToken = new Danger(this.currLino, this.currChno, Danger.ERROR_SEVERITY, Danger.EFLOAT_PARSE_NUM, "Expected decimal integer after exponent specifier");
                        this.errs.Add(errToken);
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

                    tryFloat32 = (float)tryFloat64;
                    if (tryFloat32 == tryFloat64)
                    {
                        newToken.type = TokenType.VAL_FLOAT32;
                        newToken.data = tryFloat32;
                    }
                    else
                    {
                        newToken.type = TokenType.VAL_FLOAT64;
                        newToken.data = tryFloat64;
                    }
                    this.tokens.Add(newToken);
                }
                catch (OverflowException)
                {
                    newToken.type = TokenType.VAL_FLOAT64;
                    newToken.data = Double.MaxValue;
                    this.tokens.Add(newToken);
                    this.errs.Add(new Danger(newToken.lino, newToken.chno, Danger.WFLOAT_OVFLOW_SEV, Danger.WFLOAT_OVFLOW_NUM, "Float here overflowed. Defaulting to FLOAT64_MAX."));
                    while (currChar == '_' || Char.IsLetterOrDigit(currChar))
                    {
                        this.EatChar();
                        currChar = this.PeekChar();
                    }
                    return currChar;
                }
            }
            else
            {
                if (num.Length == 0)
                {
                    Danger errToken = new Danger(this.currLino, this.currChno, Danger.ERROR_SEVERITY, Danger.EINTEGER_PARSE_NUM, "Number has no digits after base");
                    this.errs.Add(errToken);
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
                    this.errs.Add(new Danger(newToken.lino, newToken.chno, Danger.WINTEGER_OVFLOW_SEV, Danger.WINTEGER_OVFLOW_NUM, "Integer overflow here. Defaulting to UINT64_MAX"));
                    newToken.type = TokenType.WARNING;
                    newToken.data = UInt64.MaxValue;
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
            Token newToken = new Token(TokenType.VAL_STRING, this.currLino, this.currChno, null);

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
                    this.errs.Add(new Danger(this.currLino, this.currChno, Danger.ERROR_SEVERITY, Danger.ESTRING_PARSE_NUM, message));
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
                        Danger errToken = new Danger(this.currLino, this.currChno, Danger.ERROR_SEVERITY, Danger.EESCAPE_PARSE_NUM, "Expected 2 hex digits for \\x escape");
                        this.errs.Add(errToken);
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
                else if ((index = this.StartsWithIndex(Lexer.DelimiterStrings)) >= 0)
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
                    Danger newErr = new Danger(this.currLino, this.currChno, Danger.ERROR_SEVERITY, Danger.EUNRECOGNIZABLE_CHAR_NUM, "Unexpected Character");
                    this.EatChar();
                    currChar = this.PeekChar();
                    this.errs.Add(newErr);
                }
            }

            this.tokens.Add(new Token(TokenType.STREAM_TERM, this.currLino, this.currChno, null));
        }
    }
}
