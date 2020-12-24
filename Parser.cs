using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Foundations
{
    enum DerivationType
    {
        IDENTIFIER,
        UINT64,
        FLOAT32,
        FLOAT64,
        STRING,

        BINARY_OPERATOR,
        POSTFIX_UNARY_OPERATOR,
        PREFIX_UNARY_OPERATOR,
        LEFT_PARA,
        RIGHT_PARA,
        PARA_EXPR,
        BINARY_TREE,
        POSTFIX_UNARY_TREE,
        PREFIX_UNARY_TREE,

        LEFT_CURLY,
        RIGHT_CURLY,
        LEFT_SQUARE,
        RIGHT_SQUARE,
        STATEMENT,
        ARRAY_LITERAL,

        ROOT
    }

    class BinOpTree
    {
        public TokenType op;
        public int priority;
        public DerivationTree lchild;
        public DerivationTree rchild;
    }

    class PreUnaryTree
    {
        public TokenType op;
        public int priority;
        public DerivationTree child;
    }

    class PostUnaryTree
    {
        public TokenType op;
        public int priority;
        public DerivationTree child;
    }

    class StatementTree
    {
        public DerivationTree expr;
        public int flags;
        public StatementTree children;
        public StatementTree next;
    }

    class DerivationTree
    {
        public DerivationType type;
        public int lino;
        public int chno;
        public object data;

        public DerivationTree(DerivationType type, int lino, int chno, object data)
        {
            this.data = data;
            this.lino = lino;
            this.chno = chno;
            this.type = type;
        }
    }

    class Parser
    {
        public DerivationTree root;
        public List<Danger> dangers;
        public Token[] tokens;
        public int tokensptr;

        public Parser(Token[] toks)
        {
            this.root = new DerivationTree(DerivationType.ROOT, toks[0].lino, toks[0].chno, null);
            this.dangers = new List<Danger>();
            this.tokens = toks;
            this.tokensptr = 0;
        }
    }
}
