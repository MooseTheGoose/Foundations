using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Foundations
{
    class Danger
    {
        public int lino;
        public int chno;
        public int severity;
        public int num; 
        public string message;

        public const int ERROR_SEVERITY = -1;

        public const int EINTEGER_PARSE_NUM = 0x1002;
        public const int EFLOAT_PARSE_NUM = 0x1003;
        public const int ESTRING_PARSE_NUM = 0x1004;
        public const int EESCAPE_PARSE_NUM = 0x1005;
        public const int EUNRECOGNIZABLE_CHAR_NUM = 0x1006;

        public const int WFLOAT_OVFLOW_NUM = 0x1000;
        public const int WINTEGER_OVFLOW_NUM = 0x1001;

        public const int WFLOAT_OVFLOW_SEV = 0x1000;
        public const int WINTEGER_OVFLOW_SEV = 0x1000;

        public Danger(int lino, int chno, int severity, int num, string message)
        {
            this.lino = lino;
            this.chno = chno;
            this.severity = severity;
            this.message = message;
        }
    }
}
