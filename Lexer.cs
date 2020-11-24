using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Foundations
{
    enum TokenType 
    {
        TERM = 0
    }

    enum TokenOperatorType 
    {
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
        /*
         * Source file lines
         */
        public int curr_lino;
        public int curr_chno;
        public string[] lines;

        /*
         * Preprocessor lines
         */
        public int curr_pre_lino;
        public int curr_pre_chno;
        public string[] pre_lines;

        public Lexer(string src)
        {
            this.curr_lino = 1;
            this.curr_chno = 1;

            List<string> readLines = new List<string>();
            using (StringReader sr = new StringReader(src))
            {
                string curr_line = sr.ReadLine();
                while (curr_line != null)
                {
                    readLines.Add(curr_line);
                }
            }

            this.lines = readLines.ToArray();

            this.curr_pre_lino = 1;
            this.curr_pre_chno = 1;
            pre_lines = null;
        }

        public int EatChar()
        {
            int ch = '\0';

            if (pre_lines != null)
            {
                string curr_line = this.pre_lines[this.curr_pre_lino - 1];

                if (curr_line.Length < this.curr_pre_chno)
                {
                    if (this.pre_lines.Length > this.curr_pre_lino)
                    {
                        this.curr_pre_lino += 1;
                        this.curr_pre_chno = 1;
                        ch = '\n';
                    }
                    else
                    {
                        this.pre_lines = null;
                    }
                }
                else
                {
                    ch = curr_line[this.curr_pre_chno - 1];
                    this.curr_pre_chno += 1;
                }
            }

            if(pre_lines == null)
            {
                string curr_line = this.lines[this.curr_lino - 1];

                if (curr_line.Length < this.curr_chno)
                {
                    if (this.lines.Length > this.curr_lino)
                    {
                        this.curr_lino += 1;
                        this.curr_chno = 1;
                        ch = '\n';
                    }
                }
                else
                {
                    ch = curr_line[this.curr_chno - 1];
                    this.curr_chno += 1;
                }
            }

            return ch;
        }

        public int PeekChar()
        {
            int ch = 0;
            string[] temp_lines = pre_lines;

            if (pre_lines != null)
            {
                string curr_line = this.pre_lines[this.curr_pre_lino - 1];

                if (curr_line.Length < this.curr_pre_chno)
                {
                    if (this.pre_lines.Length > this.curr_pre_lino)
                    {
                        ch = '\n';
                    }
                    else
                    {
                        this.pre_lines = null;
                    }
                }
                else
                {
                    ch = curr_line[this.curr_pre_chno - 1];
                }
            }

            if (pre_lines == null)
            {
                string curr_line = this.lines[this.curr_lino - 1];

                if (curr_line.Length < this.curr_chno)
                {
                    if (this.lines.Length > this.curr_lino)
                    {
                        ch = '\n';
                    }
                }
                else
                {
                    ch = curr_line[this.curr_chno - 1];   
                }
            }    

            pre_lines = temp_lines;
            return ch;
        }


    }
}
