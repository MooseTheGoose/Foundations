using System;
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
    }

    class Lexer
    {
        public int curr_lino;
        public int curr_chno;

        public Lexer()
        {
            this.curr_lino = 1;
            this.curr_chno = 1;
        }
    }
}
