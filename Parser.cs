using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Foundations
{
    enum DerivationType
    {
        ROOT = 0,

        VAL_BASE = 0x10000,
        VAL_POSTFIX_OPTREE = 0x10001,
        VAL_PREFIX_OPTREE = 0x10002,
        VAL_BINARY_OPTREE = 0x10003,
        VAL_ARRAY = 0x10005,
        VAL_STRUCT = 0x10006,
        VAL_EXPR = 0x10007,
        VAL_PARA = 0x10008,
        VAL_BLOCK = 0x10009,
        VAL_FN_CALL = 0x1000A,

        BINARY_OPERATOR = 0x20000,
        POSTFIX_UNARY_OPERATOR = 0x20001,
        PREFIX_UNARY_OPERATOR = 0x20002,

        LEFT_CURLY = 0x30000,
        RIGHT_CURLY = 0x30001,
        LEFT_SQUARE = 0x30002,
        RIGHT_SQUARE = 0x30003,
        LEFT_PARA = 0x30006,
        RIGHT_PARA = 0x30007,

        TYPE_BASE = 0x40000,
        TYPE_COMPOSITE = 0x4000A
    }

    class DerivationTree
    {
        public DerivationType type;
        public int lino;
        public int chno;
        public DerivationTree next;
        public DerivationTree children;
        public TokenType tokentype;
        public object data;

        public const uint CategoryLimit = 0xFFFF;
        public const uint ValueBase = 0x10000;
        public const uint OperatorBase = 0x20000;
        public const uint BracketBase = 0x30000;
        public const uint TypeBase = 0x40000;

        public DerivationTree(DerivationType type, int lino, int chno, TokenType toktype, object data)
        {
            this.tokentype = toktype;
            this.data = data;
            this.lino = lino;
            this.chno = chno;
            this.type = type;
            this.next = null;
            this.children = null;
        }

        public string PrintTree()
        {
            string treerep = "()";
            switch (this.type)
            {
                case DerivationType.VAL_BASE:
                    treerep = this.data.ToString();
                    break;
                case DerivationType.TYPE_BASE:
                    treerep = Lexer.KeywordStrings[Array.IndexOf(Lexer.KeywordTypes, this.tokentype)];
                    break;
                case DerivationType.VAL_PREFIX_OPTREE:
                case DerivationType.VAL_POSTFIX_OPTREE:
                case DerivationType.VAL_BINARY_OPTREE:
                    treerep = "(" + Lexer.DelimiterStrings[Array.IndexOf(Lexer.DelimiterTypes, this.tokentype)] + " ";
                    DerivationTree child = this.children;
                    while (child != null)
                    {
                        treerep += child.PrintTree() + " ";
                        child = child.next;
                    }
                    treerep = treerep.Substring(0, treerep.Length - 1) + ")";
                    break;
                case DerivationType.BINARY_OPERATOR:
                case DerivationType.POSTFIX_UNARY_OPERATOR:
                case DerivationType.PREFIX_UNARY_OPERATOR:
                    treerep = Lexer.DelimiterStrings[Array.IndexOf(Lexer.DelimiterTypes, this.tokentype)];
                    break;
            }
            return treerep;
        }
    }

    class Parser
    {
        public DerivationTree root;
        public List<Danger> dangers;
        public Token[] tokens;
        public int tokensptr;

        public static TokenType[] PostfixOperatorTypes = {
            TokenType.OP_ADDR, TokenType.OP_DEREF
        };

        public static int[] PostfixOperatorLevels = {
            10, 10
        };

        public static TokenType[] PrefixOperatorTypes = {
            TokenType.OP_BWNOT, TokenType.OP_PLUS, TokenType.OP_MINUS,
            TokenType.OP_DEFN, TokenType.OP_ASGN
        };

        public static int[] PrefixOperatorLevels = {
            9, 9, 9, 0, 0
        };

        public static TokenType[] BinaryOperatorTypes = {
            TokenType.OP_PLUS, TokenType.OP_MINUS,
            TokenType.OP_MULT, TokenType.OP_DIV,
            TokenType.OP_MOD, TokenType.OP_BWOR,
            TokenType.OP_BWXOR, TokenType.OP_BWAND,
            TokenType.OP_DEFN, TokenType.OP_ASGN
        };

        public static int[] BinaryOperatorLevels = {
            7, 7, 8, 8, 8, 2, 3, 4, 0, 0
        };

        /*
         * Levels serve as lookaside for this buffer
         * which mentions if an operator is LR associative or not.
         */
        public static bool[] OperatorLevelLRAssociative = {
            false, true, true, true, true, true, 
            true, true, true, false, true, true
        };

        public Parser(Token[] toks)
        {
            this.root = new DerivationTree(DerivationType.ROOT, toks[0].lino, toks[0].chno, toks[0].type, null);
            this.dangers = new List<Danger>();
            this.tokens = toks;
            this.tokensptr = 0;
        }

        public Token PeekToken()
        {
            return this.tokens[this.tokensptr];
        }

        public Token EatToken() 
        {
            Token t = this.tokens[this.tokensptr];
            if (this.tokensptr + 1 < this.tokens.Length) 
            {
                this.tokensptr += 1;
            }
            return t;
        }

        public static string PrintStack(List<DerivationTree> currentStack)
        {
            StringBuilder stackrep = new StringBuilder("[");
            for (int i = 0; i < currentStack.Count - 1; i++)
            {
                stackrep.Append(currentStack[i].PrintTree());
                stackrep.Append(", ");
            }
            if (currentStack.Count > 0)
            {
                stackrep.Append(currentStack[currentStack.Count - 1].PrintTree());
            }
            stackrep.Append("]");
            return stackrep.ToString();
        }

        /*
         * When calling, t.type must not initially be a terminator (newline, statement, stream).
         */
        public DerivationTree ParseExpression() 
        {
            Token t = this.PeekToken();
            List<DerivationTree> exprStack = new List<DerivationTree>();
            DerivationTree exprtree = new DerivationTree(DerivationType.VAL_EXPR, t.lino, t.chno, t.type, null);
            int currentLevel = Int32.MinValue;

            while (!((uint)t.type >= Token.StatementTermBase && (uint)t.type <= Token.StatementTermBase + Token.CategoryLimit))
            {
                /* 
                 * If the token is a value type or a type type (if that makes sense), 
                 * then just put it on the stack and go to the next token.
                 */
                if ((uint)t.type >= Token.ValueBase && (uint)t.type <= Token.ValueBase + Token.CategoryLimit)
                {
                    DerivationTree valuetree = new DerivationTree(DerivationType.VAL_BASE, t.lino, t.chno, t.type, t.data);
                    exprStack.Add(valuetree);
                    this.EatToken();
                    t = this.PeekToken();
                }
                else if ((uint)t.type >= Token.TypeBase && (uint)t.type <= Token.TypeBase + Token.CategoryLimit)
                {
                    DerivationTree typetree = new DerivationTree(DerivationType.TYPE_BASE, t.lino, t.chno, t.type, t.data);
                    exprStack.Add(typetree);
                    this.EatToken();
                    t = this.PeekToken();
                }

                /*
                 * If the token is an operator, find out whether it's postfix, prefix, or binary.
                 * (Any ternary operators will be binary operators in the same precedence level).
                 */
                else if ((uint)t.type >= Token.OperatorBase && (uint)t.type <= Token.OperatorBase + Token.CategoryLimit)
                {
                    this.EatToken();
                    Token lookahead = this.PeekToken();
                    DerivationTree optree = new DerivationTree(DerivationType.BINARY_OPERATOR, t.lino, t.chno, t.type, null);

                    /*
                     * If the top of the stack is not an operator or is postfix, 
                     * then this is either postfix or binary.
                     */
                    if (exprStack.Count > 0
                        && (!((uint)exprStack[exprStack.Count - 1].tokentype >= Token.OperatorBase
                             && (uint)exprStack[exprStack.Count - 1].tokentype <= Token.OperatorBase + Token.CategoryLimit)
                            || exprStack[exprStack.Count-1].type == DerivationType.POSTFIX_UNARY_OPERATOR))
                    {
                        /*
                         * If the token ahead is not an operator besides a prefix nor a statement terminator,
                         * then check whether it's binary or postfix (Note that an operator cannot be both
                         * postfix and binary in this language, since that runs into ambiguities. 
                         * Example, suppose ! is postfix and binary,
                         * and + is prefix and binary. Then A!! + 5 is ambiguous).
                         */
                        if ((!((uint)lookahead.type >= Token.OperatorBase && (uint)lookahead.type <= Token.OperatorBase + Token.CategoryLimit) 
                            || Parser.PrefixOperatorTypes.Contains(lookahead.type))
                               && !((uint)lookahead.type >= Token.StatementTermBase && (uint)lookahead.type <= Token.StatementTermBase + Token.CategoryLimit)
                               && Parser.BinaryOperatorTypes.Contains(t.type))
                        {
                            int levelLookaside = Array.IndexOf(Parser.BinaryOperatorTypes, t.type);

                            if (levelLookaside >= 0)
                            {
                                optree.type = DerivationType.BINARY_OPERATOR;
                                optree.data = Parser.BinaryOperatorLevels[levelLookaside];

                                if ((int)optree.data < currentLevel || (int)optree.data == currentLevel && Parser.OperatorLevelLRAssociative[currentLevel])
                                {
                                    Console.WriteLine("Encountered operator with less precedence. Must resolve operators with greater precedence...");
                                    int status = this.ResolveOperators(exprStack, (int)optree.data, Parser.OperatorLevelLRAssociative[currentLevel]);
                                    if (status != 0)
                                    {
                                        t = lookahead;
                                        while (!((uint)t.type >= Token.StatementTermBase && (uint)t.type <= Token.StatementTermBase + Token.CategoryLimit))
                                        {
                                            this.EatToken();
                                            t = this.PeekToken();
                                        }
                                        return exprtree;
                                    }
                                }

                                currentLevel = (int)optree.data;
                                exprStack.Add(optree);
                            }
                        }
                        else
                        {
                            int levelLookaside = Array.IndexOf(Parser.PostfixOperatorTypes, t.type);

                            if (levelLookaside >= 0)
                            {
                                optree.type = DerivationType.POSTFIX_UNARY_OPERATOR;
                                optree.data = Parser.PostfixOperatorLevels[levelLookaside];

                                if ((int)optree.data < currentLevel || (int)optree.data == currentLevel && Parser.OperatorLevelLRAssociative[currentLevel])
                                {
                                    Console.WriteLine("Encountered operator with less precedence. Must resolve operators with greater precedence...");
                                    int status = this.ResolveOperators(exprStack, (int)optree.data, Parser.OperatorLevelLRAssociative[currentLevel]);
                                    if (status != 0)
                                    {
                                        t = lookahead;
                                        while (!((uint)t.type >= Token.StatementTermBase && (uint)t.type <= Token.StatementTermBase + Token.CategoryLimit))
                                        {
                                            this.EatToken();
                                            t = this.PeekToken();
                                        }
                                        return exprtree;
                                    }
                                }

                                currentLevel = (int)optree.data;
                                exprStack.Add(optree);
                            }
                            else
                            {
                                /*
                                 * Note that at this point, you might want to check if you see '++' or '--'
                                 * here to give a more appropriate warning.
                                 */
                                Danger err = new Danger(t.lino, t.chno, Danger.ERROR_SEVERITY, Danger.EPOSTOP_EXPECTED, "Expected postfix operator here");
                                this.dangers.Add(err);
                                t = lookahead;
                                while (!((uint)t.type >= Token.StatementTermBase && (uint)t.type <= Token.StatementTermBase + Token.CategoryLimit))
                                {
                                    this.EatToken();
                                    t = this.PeekToken();
                                }
                                return exprtree;
                            }
                        }
                    }
                    /*
                     * If the stack is empty or the top of the stack is an operator,
                     * and the token ahead is not an operator or is prefix, then it must be prefix.
                     */
                    else if ((exprStack.Count == 0 
                             || (uint)exprStack[exprStack.Count - 1].tokentype >= Token.OperatorBase 
                                   && (uint)exprStack[exprStack.Count-1].tokentype <= Token.OperatorBase + Token.CategoryLimit)
                        && (!((uint)lookahead.type >= Token.OperatorBase && (uint)lookahead.type <= Token.OperatorBase + Token.CategoryLimit)
                            || Parser.PrefixOperatorTypes.Contains(lookahead.type)))
                    {
                        int levelLookaside = Array.IndexOf(Parser.PrefixOperatorTypes, t.type);

                        if (levelLookaside >= 0)
                        {
                            /*
                             *  Note that at this point, you might want to give a warning
                             *  if you see '--' or '++', since we don't do that here.
                             */
                            optree.type = DerivationType.PREFIX_UNARY_OPERATOR;
                            optree.data = Parser.PrefixOperatorLevels[levelLookaside];

                            if ((int)optree.data < currentLevel || (int)optree.data == currentLevel && Parser.OperatorLevelLRAssociative[currentLevel])
                            {
                                int status = this.ResolveOperators(exprStack, (int)optree.data, Parser.OperatorLevelLRAssociative[currentLevel]);
                                if (status != 0)
                                {
                                    return exprtree;
                                }
                            }

                            currentLevel = (int)optree.data;
                            exprStack.Add(optree);
                        }
                        else
                        {
                            Danger err = new Danger(t.lino, t.chno, Danger.ERROR_SEVERITY, Danger.EPREOP_EXPECTED, "Expected prefix operator here");
                            this.dangers.Add(err);
                            t = lookahead;
                            while (!((uint)t.type >= Token.StatementTermBase && (uint)t.type <= Token.StatementTermBase + Token.CategoryLimit))
                            {
                                this.EatToken();
                                t = this.PeekToken();
                            }
                            return exprtree;
                        }
                    }
                    t = lookahead;
                }
                Console.WriteLine("Current Stack: " + PrintStack(exprStack));
                Console.ReadKey();
            }

            int finalStatus = this.ResolveOperators(exprStack, -1, true);
            if (finalStatus == 0 && exprStack.Count > 0)
            {
                exprtree.children = exprStack[0];
            }
            return exprtree;
        }

        public int ResolveOperators(List<DerivationTree> exprStack, int level, bool isLR)
        {
            int currLevel = Int32.MaxValue;
            while (exprStack.Count > 1
                && (currLevel > level || currLevel == level && isLR))
            {
                DerivationTree currTree = exprStack[exprStack.Count - 1];
                DerivationTree prevTree = exprStack[exprStack.Count - 2];

                if (currTree.type == DerivationType.POSTFIX_UNARY_OPERATOR)
                {
                    currLevel = (int)currTree.data;
                    if (currLevel > level || currLevel == level && isLR)
                    {
                        DerivationTree valTree = new DerivationTree(DerivationType.VAL_POSTFIX_OPTREE, currTree.lino, currTree.chno, currTree.tokentype, null);
                        valTree.children = prevTree;

                        exprStack.RemoveAt(exprStack.Count - 1);
                        exprStack.RemoveAt(exprStack.Count - 1);
                        exprStack.Add(valTree);
                    }
                }
                else if (prevTree.type == DerivationType.PREFIX_UNARY_OPERATOR)
                {
                    currLevel = (int)prevTree.data;

                    if (currLevel > level || currLevel == level && isLR)
                    {
                        DerivationTree valTree = new DerivationTree(DerivationType.VAL_PREFIX_OPTREE, prevTree.lino, prevTree.chno, prevTree.tokentype, null);
                        valTree.children = currTree;

                        exprStack.RemoveAt(exprStack.Count - 1);
                        exprStack.RemoveAt(exprStack.Count - 1);
                        exprStack.Add(valTree);
                    }
                }
                else if (prevTree.type == DerivationType.BINARY_OPERATOR)
                {
                    currLevel = (int)prevTree.data;

                    if (currLevel > level || currLevel == level && isLR)
                    {
                        DerivationTree valTree = new DerivationTree(DerivationType.VAL_BINARY_OPTREE, prevTree.lino, prevTree.chno, prevTree.tokentype, null);
                        valTree.children = exprStack[exprStack.Count - 3];
                        valTree.children.next = currTree;

                        exprStack.RemoveAt(exprStack.Count - 1);
                        exprStack.RemoveAt(exprStack.Count - 1);
                        exprStack.RemoveAt(exprStack.Count - 1);
                        exprStack.Add(valTree);
                    }
                }
                else
                {
                    Danger error = new Danger(prevTree.lino, prevTree.chno, Danger.ERROR_SEVERITY, Danger.EOPERATOR_EXPECTED, "Expected operator here or postfix operator after");
                    this.dangers.Add(error);
                    while (exprStack.Count > 1)
                    {
                        exprStack.RemoveAt(exprStack.Count - 1);
                    }
                    return -1;
                }
            }

            return 0;
        }
    }
}
