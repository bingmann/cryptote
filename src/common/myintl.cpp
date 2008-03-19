// $Id$

// Custom Implementation of a locale class based on wxLocale's source from
// wxWidgets 2.8.7. The class MyLocale is derived from wxLocale and implements
// lookups from gettext catalogs stored in memory. This way no external .mo
// files are required, which are problematic on Windows.

/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/intl.cpp
// Purpose:     Internationalization and localisation for wxWidgets
// Author:      Vadim Zeitlin
// Modified by: Michael N. Filippov <michael@idisys.iae.nsk.su>
//              (2003/09/30 - PluralForms support)
// Created:     29/01/98
// RCS-ID:      $Id$
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "myintl.h"

#include "wx/log.h"
#include "wx/hashmap.h"
#include "wx/utils.h"
#include "wx/ptr_scpd.h"

// ----------------------------------------------------------------------------
// simple types
// ----------------------------------------------------------------------------

// this should *not* be wxChar, this type must have exactly 8 bits!
typedef wxUint8 size_t8;
typedef wxUint32 size_t32;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// magic number identifying the .mo format file
const size_t32 MSGCATALOG_MAGIC    = 0x950412de;
const size_t32 MSGCATALOG_MAGIC_SW = 0xde120495;

// ----------------------------------------------------------------------------
// Plural forms parser
// ----------------------------------------------------------------------------

/*
                                Simplified Grammar

Expression:
    LogicalOrExpression '?' Expression ':' Expression
    LogicalOrExpression

LogicalOrExpression:
    LogicalAndExpression "||" LogicalOrExpression   // to (a || b) || c
    LogicalAndExpression

LogicalAndExpression:
    EqualityExpression "&&" LogicalAndExpression    // to (a && b) && c
    EqualityExpression

EqualityExpression:
    RelationalExpression "==" RelationalExperession
    RelationalExpression "!=" RelationalExperession
    RelationalExpression

RelationalExpression:
    MultiplicativeExpression '>' MultiplicativeExpression
    MultiplicativeExpression '<' MultiplicativeExpression
    MultiplicativeExpression ">=" MultiplicativeExpression
    MultiplicativeExpression "<=" MultiplicativeExpression
    MultiplicativeExpression

MultiplicativeExpression:
    PmExpression '%' PmExpression
    PmExpression

PmExpression:
    N
    Number
    '(' Expression ')'
*/

class MyPluralFormsToken
{
public:
    enum Type
    {
        T_ERROR, T_EOF, T_NUMBER, T_N, T_PLURAL, T_NPLURALS, T_EQUAL, T_ASSIGN,
        T_GREATER, T_GREATER_OR_EQUAL, T_LESS, T_LESS_OR_EQUAL,
        T_REMINDER, T_NOT_EQUAL,
        T_LOGICAL_AND, T_LOGICAL_OR, T_QUESTION, T_COLON, T_SEMICOLON,
        T_LEFT_BRACKET, T_RIGHT_BRACKET
    };
    Type type() const { return m_type; }
    void setType(Type type) { m_type = type; }
    // for T_NUMBER only
    typedef int Number;
    Number number() const { return m_number; }
    void setNumber(Number num) { m_number = num; }
private:
    Type m_type;
    Number m_number;
};

class MyPluralFormsScanner
{
public:
    MyPluralFormsScanner(const char* s);
    const MyPluralFormsToken& token() const { return m_token; }
    bool nextToken();  // returns false if error
private:
    const char* m_s;
    MyPluralFormsToken m_token;
};

MyPluralFormsScanner::MyPluralFormsScanner(const char* s) : m_s(s)
{
    nextToken();
}

bool MyPluralFormsScanner::nextToken()
{
    MyPluralFormsToken::Type type = MyPluralFormsToken::T_ERROR;
    while (isspace(*m_s))
    {
        ++m_s;
    }
    if (*m_s == 0)
    {
        type = MyPluralFormsToken::T_EOF;
    }
    else if (isdigit(*m_s))
    {
        MyPluralFormsToken::Number number = *m_s++ - '0';
        while (isdigit(*m_s))
        {
            number = number * 10 + (*m_s++ - '0');
        }
        m_token.setNumber(number);
        type = MyPluralFormsToken::T_NUMBER;
    }
    else if (isalpha(*m_s))
    {
        const char* begin = m_s++;
        while (isalnum(*m_s))
        {
            ++m_s;
        }
        size_t size = m_s - begin;
        if (size == 1 && memcmp(begin, "n", size) == 0)
        {
            type = MyPluralFormsToken::T_N;
        }
        else if (size == 6 && memcmp(begin, "plural", size) == 0)
        {
            type = MyPluralFormsToken::T_PLURAL;
        }
        else if (size == 8 && memcmp(begin, "nplurals", size) == 0)
        {
            type = MyPluralFormsToken::T_NPLURALS;
        }
    }
    else if (*m_s == '=')
    {
        ++m_s;
        if (*m_s == '=')
        {
            ++m_s;
            type = MyPluralFormsToken::T_EQUAL;
        }
        else
        {
            type = MyPluralFormsToken::T_ASSIGN;
        }
    }
    else if (*m_s == '>')
    {
        ++m_s;
        if (*m_s == '=')
        {
            ++m_s;
            type = MyPluralFormsToken::T_GREATER_OR_EQUAL;
        }
        else
        {
            type = MyPluralFormsToken::T_GREATER;
        }
    }
    else if (*m_s == '<')
    {
        ++m_s;
        if (*m_s == '=')
        {
            ++m_s;
            type = MyPluralFormsToken::T_LESS_OR_EQUAL;
        }
        else
        {
            type = MyPluralFormsToken::T_LESS;
        }
    }
    else if (*m_s == '%')
    {
        ++m_s;
        type = MyPluralFormsToken::T_REMINDER;
    }
    else if (*m_s == '!' && m_s[1] == '=')
    {
        m_s += 2;
        type = MyPluralFormsToken::T_NOT_EQUAL;
    }
    else if (*m_s == '&' && m_s[1] == '&')
    {
        m_s += 2;
        type = MyPluralFormsToken::T_LOGICAL_AND;
    }
    else if (*m_s == '|' && m_s[1] == '|')
    {
        m_s += 2;
        type = MyPluralFormsToken::T_LOGICAL_OR;
    }
    else if (*m_s == '?')
    {
        ++m_s;
        type = MyPluralFormsToken::T_QUESTION;
    }
    else if (*m_s == ':')
    {
        ++m_s;
        type = MyPluralFormsToken::T_COLON;
    } else if (*m_s == ';') {
        ++m_s;
        type = MyPluralFormsToken::T_SEMICOLON;
    }
    else if (*m_s == '(')
    {
        ++m_s;
        type = MyPluralFormsToken::T_LEFT_BRACKET;
    }
    else if (*m_s == ')')
    {
        ++m_s;
        type = MyPluralFormsToken::T_RIGHT_BRACKET;
    }
    m_token.setType(type);
    return type != MyPluralFormsToken::T_ERROR;
}

class MyPluralFormsNode;

// NB: Can't use wxDEFINE_SCOPED_PTR_TYPE because MyPluralFormsNode is not
//     fully defined yet:
class MyPluralFormsNodePtr
{
public:
    MyPluralFormsNodePtr(MyPluralFormsNode *p = NULL) : m_p(p) {}
    ~MyPluralFormsNodePtr();
    MyPluralFormsNode& operator*() const { return *m_p; }
    MyPluralFormsNode* operator->() const { return m_p; }
    MyPluralFormsNode* get() const { return m_p; }
    MyPluralFormsNode* release();
    void reset(MyPluralFormsNode *p);

private:
    MyPluralFormsNode *m_p;
};

class MyPluralFormsNode
{
public:
    MyPluralFormsNode(const MyPluralFormsToken& token) : m_token(token) {}
    const MyPluralFormsToken& token() const { return m_token; }
    const MyPluralFormsNode* node(size_t i) const { return m_nodes[i].get(); }
    void setNode(size_t i, MyPluralFormsNode* n);
    MyPluralFormsNode* releaseNode(size_t i);
    MyPluralFormsToken::Number evaluate(MyPluralFormsToken::Number n) const;

private:
    MyPluralFormsToken m_token;
    MyPluralFormsNodePtr m_nodes[3];
};

MyPluralFormsNodePtr::~MyPluralFormsNodePtr()
{
    delete m_p;
}

MyPluralFormsNode* MyPluralFormsNodePtr::release()
{
    MyPluralFormsNode *p = m_p;
    m_p = NULL;
    return p;
}

void MyPluralFormsNodePtr::reset(MyPluralFormsNode *p)
{
    if (p != m_p)
    {
        delete m_p;
        m_p = p;
    }
}

void MyPluralFormsNode::setNode(size_t i, MyPluralFormsNode* n)
{
    m_nodes[i].reset(n);
}

MyPluralFormsNode*  MyPluralFormsNode::releaseNode(size_t i)
{
    return m_nodes[i].release();
}

MyPluralFormsToken::Number
MyPluralFormsNode::evaluate(MyPluralFormsToken::Number n) const
{
    switch (token().type())
    {
        // leaf
        case MyPluralFormsToken::T_NUMBER:
            return token().number();
        case MyPluralFormsToken::T_N:
            return n;
        // 2 args
        case MyPluralFormsToken::T_EQUAL:
            return node(0)->evaluate(n) == node(1)->evaluate(n);
        case MyPluralFormsToken::T_NOT_EQUAL:
            return node(0)->evaluate(n) != node(1)->evaluate(n);
        case MyPluralFormsToken::T_GREATER:
            return node(0)->evaluate(n) > node(1)->evaluate(n);
        case MyPluralFormsToken::T_GREATER_OR_EQUAL:
            return node(0)->evaluate(n) >= node(1)->evaluate(n);
        case MyPluralFormsToken::T_LESS:
            return node(0)->evaluate(n) < node(1)->evaluate(n);
        case MyPluralFormsToken::T_LESS_OR_EQUAL:
            return node(0)->evaluate(n) <= node(1)->evaluate(n);
        case MyPluralFormsToken::T_REMINDER:
            {
                MyPluralFormsToken::Number number = node(1)->evaluate(n);
                if (number != 0)
                {
                    return node(0)->evaluate(n) % number;
                }
                else
                {
                    return 0;
                }
            }
        case MyPluralFormsToken::T_LOGICAL_AND:
            return node(0)->evaluate(n) && node(1)->evaluate(n);
        case MyPluralFormsToken::T_LOGICAL_OR:
            return node(0)->evaluate(n) || node(1)->evaluate(n);
        // 3 args
        case MyPluralFormsToken::T_QUESTION:
            return node(0)->evaluate(n)
                ? node(1)->evaluate(n)
                : node(2)->evaluate(n);
        default:
            return 0;
    }
}

class MyPluralFormsCalculator
{
public:
    MyPluralFormsCalculator() : m_nplurals(0), m_plural(0) {}

    // input: number, returns msgstr index
    int evaluate(int n) const;

    // input: text after "Plural-Forms:" (e.g. "nplurals=2; plural=(n != 1);"),
    // if s == 0, creates default handler
    // returns 0 if error
    static MyPluralFormsCalculator* make(const char* s = 0);

    ~MyPluralFormsCalculator() {}

    void  init(MyPluralFormsToken::Number nplurals, MyPluralFormsNode* plural);

private:
    MyPluralFormsToken::Number m_nplurals;
    MyPluralFormsNodePtr m_plural;
};

wxDEFINE_SCOPED_PTR_TYPE(MyPluralFormsCalculator)

void MyPluralFormsCalculator::init(MyPluralFormsToken::Number nplurals,
                                   MyPluralFormsNode* plural)
{
    m_nplurals = nplurals;
    m_plural.reset(plural);
}

int MyPluralFormsCalculator::evaluate(int n) const
{
    if (m_plural.get() == 0)
    {
        return 0;
    }
    MyPluralFormsToken::Number number = m_plural->evaluate(n);
    if (number < 0 || number > m_nplurals)
    {
        return 0;
    }
    return number;
}

class MyPluralFormsParser
{
public:
    MyPluralFormsParser(MyPluralFormsScanner& scanner) : m_scanner(scanner) {}
    bool parse(MyPluralFormsCalculator& rCalculator);

private:
    MyPluralFormsNode* parsePlural();
    // stops at T_SEMICOLON, returns 0 if error
    MyPluralFormsScanner& m_scanner;
    const MyPluralFormsToken& token() const;
    bool nextToken();

    MyPluralFormsNode* expression();
    MyPluralFormsNode* logicalOrExpression();
    MyPluralFormsNode* logicalAndExpression();
    MyPluralFormsNode* equalityExpression();
    MyPluralFormsNode* multiplicativeExpression();
    MyPluralFormsNode* relationalExpression();
    MyPluralFormsNode* pmExpression();
};

bool MyPluralFormsParser::parse(MyPluralFormsCalculator& rCalculator)
{
    if (token().type() != MyPluralFormsToken::T_NPLURALS)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != MyPluralFormsToken::T_ASSIGN)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != MyPluralFormsToken::T_NUMBER)
        return false;
    MyPluralFormsToken::Number nplurals = token().number();
    if (!nextToken())
        return false;
    if (token().type() != MyPluralFormsToken::T_SEMICOLON)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != MyPluralFormsToken::T_PLURAL)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != MyPluralFormsToken::T_ASSIGN)
        return false;
    if (!nextToken())
        return false;
    MyPluralFormsNode* plural = parsePlural();
    if (plural == 0)
        return false;
    if (token().type() != MyPluralFormsToken::T_SEMICOLON)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != MyPluralFormsToken::T_EOF)
        return false;
    rCalculator.init(nplurals, plural);
    return true;
}

MyPluralFormsNode* MyPluralFormsParser::parsePlural()
{
    MyPluralFormsNode* p = expression();
    if (p == NULL)
    {
        return NULL;
    }
    MyPluralFormsNodePtr n(p);
    if (token().type() != MyPluralFormsToken::T_SEMICOLON)
    {
        return NULL;
    }
    return n.release();
}

const MyPluralFormsToken& MyPluralFormsParser::token() const
{
    return m_scanner.token();
}

bool MyPluralFormsParser::nextToken()
{
    if (!m_scanner.nextToken())
        return false;
    return true;
}

MyPluralFormsNode* MyPluralFormsParser::expression()
{
    MyPluralFormsNode* p = logicalOrExpression();
    if (p == NULL)
        return NULL;
    MyPluralFormsNodePtr n(p);
    if (token().type() == MyPluralFormsToken::T_QUESTION)
    {
        MyPluralFormsNodePtr qn(new MyPluralFormsNode(token()));
        if (!nextToken())
        {
            return 0;
        }
        p = expression();
        if (p == 0)
        {
            return 0;
        }
        qn->setNode(1, p);
        if (token().type() != MyPluralFormsToken::T_COLON)
        {
            return 0;
        }
        if (!nextToken())
        {
            return 0;
        }
        p = expression();
        if (p == 0)
        {
            return 0;
        }
        qn->setNode(2, p);
        qn->setNode(0, n.release());
        return qn.release();
    }
    return n.release();
}

MyPluralFormsNode*MyPluralFormsParser::logicalOrExpression()
{
    MyPluralFormsNode* p = logicalAndExpression();
    if (p == NULL)
        return NULL;
    MyPluralFormsNodePtr ln(p);
    if (token().type() == MyPluralFormsToken::T_LOGICAL_OR)
    {
        MyPluralFormsNodePtr un(new MyPluralFormsNode(token()));
        if (!nextToken())
        {
            return 0;
        }
        p = logicalOrExpression();
        if (p == 0)
        {
            return 0;
        }
        MyPluralFormsNodePtr rn(p);    // right
        if (rn->token().type() == MyPluralFormsToken::T_LOGICAL_OR)
        {
            // see logicalAndExpression comment
            un->setNode(0, ln.release());
            un->setNode(1, rn->releaseNode(0));
            rn->setNode(0, un.release());
            return rn.release();
        }


        un->setNode(0, ln.release());
        un->setNode(1, rn.release());
        return un.release();
    }
    return ln.release();
}

MyPluralFormsNode* MyPluralFormsParser::logicalAndExpression()
{
    MyPluralFormsNode* p = equalityExpression();
    if (p == NULL)
        return NULL;
    MyPluralFormsNodePtr ln(p);   // left
    if (token().type() == MyPluralFormsToken::T_LOGICAL_AND)
    {
        MyPluralFormsNodePtr un(new MyPluralFormsNode(token()));  // up
        if (!nextToken())
        {
            return NULL;
        }
        p = logicalAndExpression();
        if (p == 0)
        {
            return NULL;
        }
        MyPluralFormsNodePtr rn(p);    // right
        if (rn->token().type() == MyPluralFormsToken::T_LOGICAL_AND)
        {
// transform 1 && (2 && 3) -> (1 && 2) && 3
//     u                  r
// l       r     ->   u      3
//       2   3      l   2
            un->setNode(0, ln.release());
            un->setNode(1, rn->releaseNode(0));
            rn->setNode(0, un.release());
            return rn.release();
        }

        un->setNode(0, ln.release());
        un->setNode(1, rn.release());
        return un.release();
    }
    return ln.release();
}

MyPluralFormsNode* MyPluralFormsParser::equalityExpression()
{
    MyPluralFormsNode* p = relationalExpression();
    if (p == NULL)
        return NULL;
    MyPluralFormsNodePtr n(p);
    if (token().type() == MyPluralFormsToken::T_EQUAL ||
        token().type() == MyPluralFormsToken::T_NOT_EQUAL)
    {
        MyPluralFormsNodePtr qn(new MyPluralFormsNode(token()));
        if (!nextToken())
        {
            return NULL;
        }
        p = relationalExpression();
        if (p == NULL)
        {
            return NULL;
        }
        qn->setNode(1, p);
        qn->setNode(0, n.release());
        return qn.release();
    }
    return n.release();
}

MyPluralFormsNode* MyPluralFormsParser::relationalExpression()
{
    MyPluralFormsNode* p = multiplicativeExpression();
    if (p == NULL)
        return NULL;
    MyPluralFormsNodePtr n(p);
    if (token().type() == MyPluralFormsToken::T_GREATER ||
	token().type() == MyPluralFormsToken::T_LESS ||
	token().type() == MyPluralFormsToken::T_GREATER_OR_EQUAL ||
	token().type() == MyPluralFormsToken::T_LESS_OR_EQUAL)
    {
        MyPluralFormsNodePtr qn(new MyPluralFormsNode(token()));
        if (!nextToken())
        {
            return NULL;
        }
        p = multiplicativeExpression();
        if (p == NULL)
        {
            return NULL;
        }
        qn->setNode(1, p);
        qn->setNode(0, n.release());
        return qn.release();
    }
    return n.release();
}

MyPluralFormsNode* MyPluralFormsParser::multiplicativeExpression()
{
    MyPluralFormsNode* p = pmExpression();
    if (p == NULL)
        return NULL;
    MyPluralFormsNodePtr n(p);
    if (token().type() == MyPluralFormsToken::T_REMINDER)
    {
        MyPluralFormsNodePtr qn(new MyPluralFormsNode(token()));
        if (!nextToken())
        {
            return NULL;
        }
        p = pmExpression();
        if (p == NULL)
        {
            return NULL;
        }
        qn->setNode(1, p);
        qn->setNode(0, n.release());
        return qn.release();
    }
    return n.release();
}

MyPluralFormsNode* MyPluralFormsParser::pmExpression()
{
    MyPluralFormsNodePtr n;
    if (token().type() == MyPluralFormsToken::T_N
        || token().type() == MyPluralFormsToken::T_NUMBER)
    {
        n.reset(new MyPluralFormsNode(token()));
        if (!nextToken())
        {
            return NULL;
        }
    }
    else if (token().type() == MyPluralFormsToken::T_LEFT_BRACKET) {
        if (!nextToken())
        {
            return NULL;
        }
        MyPluralFormsNode* p = expression();
        if (p == NULL)
        {
            return NULL;
        }
        n.reset(p);
        if (token().type() != MyPluralFormsToken::T_RIGHT_BRACKET)
        {
            return NULL;
        }
        if (!nextToken())
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
    return n.release();
}

MyPluralFormsCalculator* MyPluralFormsCalculator::make(const char* s)
{
    MyPluralFormsCalculatorPtr calculator(new MyPluralFormsCalculator);
    if (s != NULL)
    {
        MyPluralFormsScanner scanner(s);
        MyPluralFormsParser p(scanner);
        if (!p.parse(*calculator))
        {
            return NULL;
        }
    }
    return calculator.release();
}

// ----------------------------------------------------------------------------
// MyMsgCatalogMemory corresponds to one memory-file message catalog.
//
// This is a "low-level" class and is used only by MyMsgCatalog
// ----------------------------------------------------------------------------

WX_DECLARE_STRING_HASH_MAP(wxString, MyMessagesHash);

class MyMsgCatalogMemory
{
public:
    // ctor & dtor
    MyMsgCatalogMemory();
    ~MyMsgCatalogMemory();

    // associate this object with the catalog from memory, the memory is not
    // copied or freed.
    bool Load(const char* pData, size_t32 nDataLen,
              MyPluralFormsCalculatorPtr& rPluralFormsCalculator);

    // fills the hash with string-translation pairs
    void FillHash(MyMessagesHash& hash,
                  const wxString& msgIdCharset,
                  bool convertEncoding) const;

    // return the charset of the strings in this catalog or empty string if
    // none/unknown
    wxString GetCharset() const { return m_charset; }

private:
    // this implementation is binary compatible with GNU gettext() version 0.10

    // an entry in the string table
    struct MyMsgTableEntry
    {
	size_t32   nLen;           // length of the string
	size_t32   ofsString;      // pointer to the string
    };

    // header of a .mo file
    struct MyMsgCatalogHeader
    {
	size_t32  magic,          // offset +00:  magic id
	          revision,       //        +04:  revision
	          numStrings;     //        +08:  number of strings in the file
	size_t32  ofsOrigTable,   //        +0C:  start of original string table
	          ofsTransTable;  //        +10:  start of translated string table
	size_t32  nHashSize,      //        +14:  hash table size
	          ofsHashTable;   //        +18:  offset of hash table start
    };

    // all data is stored here, NULL if no data loaded
    const size_t8 *m_pData;

    // amount of memory pointed to by m_pData.
    size_t32 m_nSize;

    // data description
    size_t32                m_numStrings;   // number of strings in this domain
    const MyMsgTableEntry  *m_pOrigTable,   // pointer to original   strings
	                   *m_pTransTable;  //            translated

    wxString m_charset;               // from the message catalog header


    // swap the 2 halves of 32 bit integer if needed
    size_t32 Swap(size_t32 ui) const
    {
	return m_bSwapped
	    ? (ui << 24) | ((ui & 0xff00) << 8) | ((ui >> 8) & 0xff00) | (ui >> 24)
	    : ui;
    }

    const char *StringAtOfs(const MyMsgTableEntry *pTable, size_t32 n) const
    {
        const MyMsgTableEntry * const ent = pTable + n;

        // this check could fail for a corrupt message catalog
        size_t32 ofsString = Swap(ent->ofsString);
        if ( ofsString + Swap(ent->nLen) > m_nSize)
        {
            return NULL;
        }

        return (const char *)(m_pData + ofsString);
    }

    bool m_bSwapped;   // wrong endianness?

    DECLARE_NO_COPY_CLASS(MyMsgCatalogMemory);
};

// ----------------------------------------------------------------------------
// MyMsgCatalog corresponds to one loaded message catalog.
//
// This is a "low-level" class and is used only by MyLocale (that's why
// it's designed to be stored in a linked list)
// ----------------------------------------------------------------------------

class MyMsgCatalog
{
public:
    MyMsgCatalog() { m_conv = NULL; }
    ~MyMsgCatalog();

    // load the catalog from disk
    bool Load(const wxChar *szName,
	      const char* pCatalogData, size_t nCatalogDataLen,
              const wxChar *msgIdCharset = NULL, bool bConvertEncoding = false);

    // get name of the catalog
    wxString GetName() const { return m_name; }

    // get the translated string: returns NULL if not found
    const wxChar *GetString(const wxChar *sz, size_t n = size_t(-1)) const;

    // public variable pointing to the next element in a linked list (or NULL)
    MyMsgCatalog *m_pNext;

private:
    MyMessagesHash  m_messages; // all messages in the catalog
    wxString        m_name;     // name of the domain

    // the conversion corresponding to this catalog charset if we installed it
    // as the global one
    wxCSConv *m_conv;

    MyPluralFormsCalculatorPtr  m_pluralFormsCalculator;
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// MyMsgCatalogMemory class
// ----------------------------------------------------------------------------

MyMsgCatalogMemory::MyMsgCatalogMemory()
{
}

MyMsgCatalogMemory::~MyMsgCatalogMemory()
{
}

// open disk file and read in it's contents
bool MyMsgCatalogMemory::Load(const char* pData, size_t32 nDataLen,
			      MyPluralFormsCalculatorPtr& rPluralFormsCalculator)
{
    m_pData = (size_t8*)pData;

    // examine header
    bool bValid = nDataLen + (size_t)0 > sizeof(MyMsgCatalogHeader);

    MyMsgCatalogHeader *pHeader = (MyMsgCatalogHeader *)m_pData;
    if ( bValid ) {
	// we'll have to swap all the integers if it's true
	m_bSwapped = pHeader->magic == MSGCATALOG_MAGIC_SW;

	// check the magic number
	bValid = m_bSwapped || pHeader->magic == MSGCATALOG_MAGIC;
    }

    if ( !bValid ) {
	// it's either too short or has incorrect magic number
	wxLogWarning(_("Built-in data is not a valid message catalog."));
	return false;
    }

    // initialize
    m_numStrings  = Swap(pHeader->numStrings);
    m_pOrigTable  = (MyMsgTableEntry *)(m_pData +
					Swap(pHeader->ofsOrigTable));
    m_pTransTable = (MyMsgTableEntry *)(m_pData +
					Swap(pHeader->ofsTransTable));
    m_nSize = (size_t32)nDataLen;

    // now parse catalog's header and try to extract catalog charset and
    // plural forms formula from it:

    const char* headerData = StringAtOfs(m_pOrigTable, 0);
    if (headerData && headerData[0] == 0)
    {
	// Extract the charset:
	wxString header = wxString::FromAscii(StringAtOfs(m_pTransTable, 0));
	int begin = header.Find(wxT("Content-Type: text/plain; charset="));
	if (begin != wxNOT_FOUND)
	{
	    begin += 34; //strlen("Content-Type: text/plain; charset=")
	    size_t end = header.find('\n', begin);
	    if (end != size_t(-1))
	    {
		m_charset.assign(header, begin, end - begin);
		if (m_charset == wxT("CHARSET"))
		{
		    // "CHARSET" is not valid charset, but lazy translator
		    m_charset.Clear();
		}
	    }
	}
	// else: incorrectly filled Content-Type header

	// Extract plural forms:
	begin = header.Find(wxT("Plural-Forms:"));
	if (begin != wxNOT_FOUND)
	{
	    begin += 13;
	    size_t end = header.find('\n', begin);
	    if (end != size_t(-1))
	    {
		wxString pfs(header, begin, end - begin);
		MyPluralFormsCalculator* pCalculator = MyPluralFormsCalculator
		    ::make(pfs.ToAscii());
		if (pCalculator != 0)
		{
		    rPluralFormsCalculator.reset(pCalculator);
		}
		else
		{
		    wxLogVerbose(_("Cannot parse Plural-Forms:'%s'"), pfs.c_str());
		}
	    }
	}
	if (rPluralFormsCalculator.get() == NULL)
	{
	    rPluralFormsCalculator.reset(MyPluralFormsCalculator::make());
	}
    }

    // everything is fine
    return true;
}

void MyMsgCatalogMemory::FillHash(MyMessagesHash& hash,
				  const wxString& msgIdCharset,
				  bool convertEncoding) const
{
#if wxUSE_UNICODE
    // this parameter doesn't make sense, we always must convert encoding in
    // Unicode build
    convertEncoding = true;
#elif wxUSE_FONTMAP
    if ( convertEncoding )
    {
        // determine if we need any conversion at all
        wxFontEncoding encCat = wxFontMapperBase::GetEncodingFromName(m_charset);
        if ( encCat == wxLocale::GetSystemEncoding() )
        {
            // no need to convert
            convertEncoding = false;
        }
    }
#endif // wxUSE_UNICODE/wxUSE_FONTMAP

#if wxUSE_WCHAR_T
    // conversion to use to convert catalog strings to the GUI encoding
    wxMBConv *inputConv,
	     *inputConvPtr = NULL; // same as inputConv but safely deleteable
    if ( convertEncoding && !m_charset.empty() )
    {
        inputConvPtr =
	    inputConv = new wxCSConv(m_charset);
    }
    else // no need or not possible to convert the encoding
    {
#if wxUSE_UNICODE
        // we must somehow convert the narrow strings in the message catalog to
        // wide strings, so use the default conversion if we have no charset
        inputConv = wxConvCurrent;
#else // !wxUSE_UNICODE
        inputConv = NULL;
#endif // wxUSE_UNICODE/!wxUSE_UNICODE
    }

    // conversion to apply to msgid strings before looking them up: we only
    // need it if the msgids are neither in 7 bit ASCII nor in the same
    // encoding as the catalog
    wxCSConv *sourceConv = msgIdCharset.empty() || (msgIdCharset == m_charset)
	? NULL
	: new wxCSConv(msgIdCharset);

#elif wxUSE_FONTMAP
    wxASSERT_MSG( msgIdCharset.empty(),
                  _T("non-ASCII msgid languages only supported if wxUSE_WCHAR_T=1") );

    wxEncodingConverter converter;
    if ( convertEncoding )
    {
        wxFontEncoding targetEnc = wxFONTENCODING_SYSTEM;
        wxFontEncoding enc = wxFontMapperBase::Get()->CharsetToEncoding(m_charset, false);
        if ( enc == wxFONTENCODING_SYSTEM )
        {
            convertEncoding = false; // unknown encoding
        }
        else
        {
            targetEnc = wxLocale::GetSystemEncoding();
            if (targetEnc == wxFONTENCODING_SYSTEM)
            {
                wxFontEncodingArray a = wxEncodingConverter::GetPlatformEquivalents(enc);
                if (a[0] == enc)
                    // no conversion needed, locale uses native encoding
                    convertEncoding = false;
                if (a.GetCount() == 0)
                    // we don't know common equiv. under this platform
                    convertEncoding = false;
                targetEnc = a[0];
            }
        }

        if ( convertEncoding )
        {
            converter.Init(enc, targetEnc);
        }
    }
#endif // wxUSE_WCHAR_T/!wxUSE_WCHAR_T
    (void)convertEncoding; // get rid of warnings about unused parameter

    for (size_t32 i = 0; i < m_numStrings; i++)
    {
        const char *data = StringAtOfs(m_pOrigTable, i);

        wxString msgid;
#if wxUSE_UNICODE
        msgid = wxString(data, *inputConv);
#else // ASCII
#if wxUSE_WCHAR_T
	if ( inputConv && sourceConv )
	    msgid = wxString(inputConv->cMB2WC(data), *sourceConv);
	else
#endif
	    msgid = data;
#endif // wxUSE_UNICODE

        data = StringAtOfs(m_pTransTable, i);
        size_t length = Swap(m_pTransTable[i].nLen);
        size_t offset = 0;
        size_t index = 0;
        while (offset < length)
        {
            const char * const str = data + offset;

            wxString msgstr;
#if wxUSE_UNICODE
            msgstr = wxString(str, *inputConv);
#elif wxUSE_WCHAR_T
            if ( inputConv )
                msgstr = wxString(inputConv->cMB2WC(str), *wxConvUI);
            else
                msgstr = str;
#else // !wxUSE_WCHAR_T
#if wxUSE_FONTMAP
            if ( convertEncoding )
                msgstr = wxString(converter.Convert(str));
            else
#endif
                msgstr = str;
#endif // wxUSE_WCHAR_T/!wxUSE_WCHAR_T

            if ( !msgstr.empty() )
            {
                hash[index == 0 ? msgid : msgid + wxChar(index)] = msgstr;
            }

            // skip this string
            offset += strlen(str) + 1;
            ++index;
        }
    }

#if wxUSE_WCHAR_T
    delete sourceConv;
    delete inputConvPtr;
#endif // wxUSE_WCHAR_T
}

// ----------------------------------------------------------------------------
// MyMsgCatalog class
// ----------------------------------------------------------------------------

MyMsgCatalog::~MyMsgCatalog()
{
    if ( m_conv )
    {
        if ( wxConvUI == m_conv )
        {
            // we only change wxConvUI if it points to wxConvLocal so we reset
            // it back to it too
            wxConvUI = &wxConvLocal;
        }

        delete m_conv;
    }
}

bool MyMsgCatalog::Load(const wxChar *szName,
			const char* pCatalogData, size_t nCatalogDataLen,
			const wxChar *msgIdCharset, bool bConvertEncoding)
{
    MyMsgCatalogMemory memfile;

    m_name = szName;

    if ( !memfile.Load(pCatalogData, nCatalogDataLen, m_pluralFormsCalculator) )
        return false;

    memfile.FillHash(m_messages, msgIdCharset, bConvertEncoding);

#if wxUSE_WCHAR_T
    // we should use a conversion compatible with the message catalog encoding
    // in the GUI if we don't convert the strings to the current conversion but
    // as the encoding is global, only change it once, otherwise we could get
    // into trouble if we use several message catalogs with different encodings
    //
    // this is, of course, a hack but it at least allows the program to use
    // message catalogs in any encodings without asking the user to change his
    // locale
    if ( !bConvertEncoding &&
	 !memfile.GetCharset().empty() &&
	 wxConvUI == &wxConvLocal )
    {
        wxConvUI = m_conv = new wxCSConv(memfile.GetCharset());
    }
#endif // wxUSE_WCHAR_T

    return true;
}

const wxChar *MyMsgCatalog::GetString(const wxChar *sz, size_t n) const
{
    int index = 0;
    if (n != size_t(-1))
    {
        index = m_pluralFormsCalculator->evaluate(n);
    }

    MyMessagesHash::const_iterator i;
    if (index != 0)
    {
        i = m_messages.find(wxString(sz) + wxChar(index));   // plural
    }
    else
    {
        i = m_messages.find(sz);
    }

    if ( i != m_messages.end() )
    {
        return i->second.c_str();
    }
    else
        return NULL;
}

// ----------------------------------------------------------------------------
// wxLocale
// ----------------------------------------------------------------------------

MyLocale::MyLocale()
    : wxLocale()
{
    DoMyCommonInit();
}

MyLocale::MyLocale(const wxChar *szName, const wxChar *szShort,
		   const wxChar *szLocale, bool bLoadDefault, bool bConvertEncoding)
    : wxLocale(szName, szShort, szLocale, bLoadDefault, bConvertEncoding)
{
    DoMyCommonInit();
}

MyLocale::MyLocale(int language, int flags)
    : wxLocale(language, flags)
{
    DoMyCommonInit();
}

void MyLocale::DoMyCommonInit()
{
    m_pMsgCat = NULL;
}

// clean up
MyLocale::~MyLocale()
{
    // free memory
    MyMsgCatalog *pTmpCat;
    while ( m_pMsgCat != NULL ) {
        pTmpCat = m_pMsgCat;
        m_pMsgCat = m_pMsgCat->m_pNext;
        delete pTmpCat;
    }
}

// get the translation of given string in current locale
const wxChar *MyLocale::GetString(const wxChar *szOrigString,
                                  const wxChar *szDomain) const
{
    return GetString(szOrigString, szOrigString, size_t(-1), szDomain);
}

const wxChar *MyLocale::GetString(const wxChar *szOrigString,
                                  const wxChar *szOrigString2,
                                  size_t n,
                                  const wxChar *szDomain) const
{
    if ( wxIsEmpty(szOrigString) )
        return wxEmptyString;

    const wxChar *pszTrans = NULL;
    MyMsgCatalog *pMsgCat;

    if ( szDomain != NULL && szDomain[0] )
    {
        pMsgCat = FindCatalog(szDomain);

        // does the catalog exist?
        if ( pMsgCat != NULL )
            pszTrans = pMsgCat->GetString(szOrigString, n);
    }
    else
    {
        // search in all domains
        for ( pMsgCat = m_pMsgCat; pMsgCat != NULL; pMsgCat = pMsgCat->m_pNext )
        {
            pszTrans = pMsgCat->GetString(szOrigString, n);
            if ( pszTrans != NULL )   // take the first found
                break;
        }
    }

    if ( pszTrans == NULL )
    {
#ifdef __WXDEBUG__
	wxLogTrace(_T("i18n"),
		   _T("string \"%s\"[%ld] not found in %slocale '%s'."),
		   szOrigString, (long)n,
		   szDomain ? wxString::Format(_T("domain '%s' "), szDomain).c_str()
		   : _T(""),
		   GetLocale());
#endif // __WXDEBUG__

        if (n == size_t(-1))
            return szOrigString;
        else
            return n == 1 ? szOrigString : szOrigString2;
    }

    return pszTrans;
    // return wxLocale::GetString(szOrigString, szOrigString2, n, szDomain);
}

wxString MyLocale::GetHeaderValue( const wxChar* szHeader,
                                   const wxChar* szDomain ) const
{
    if ( wxIsEmpty(szHeader) )
        return wxEmptyString;

    wxChar const * pszTrans = NULL;
    MyMsgCatalog *pMsgCat;

    if ( szDomain != NULL )
    {
        pMsgCat = FindCatalog(szDomain);

        // does the catalog exist?
        if ( pMsgCat == NULL )
            return wxEmptyString;

        pszTrans = pMsgCat->GetString(wxEmptyString, (size_t)-1);
    }
    else
    {
        // search in all domains
        for ( pMsgCat = m_pMsgCat; pMsgCat != NULL; pMsgCat = pMsgCat->m_pNext )
        {
            pszTrans = pMsgCat->GetString(wxEmptyString, (size_t)-1);
            if ( pszTrans != NULL )   // take the first found
                break;
        }
    }

    if ( wxIsEmpty(pszTrans) )
      return wxEmptyString;

    wxChar const * pszFound = wxStrstr(pszTrans, szHeader);
    if ( pszFound == NULL )
      return wxEmptyString;

    pszFound += wxStrlen(szHeader) + 2 /* ': ' */;

    // Every header is separated by \n

    wxChar const * pszEndLine = wxStrchr(pszFound, wxT('\n'));
    if ( pszEndLine == NULL ) pszEndLine = pszFound + wxStrlen(pszFound);


    // wxString( wxChar*, length);
    wxString retVal( pszFound, pszEndLine - pszFound );

    return retVal;
}

// find catalog by name in a linked list, return NULL if !found
MyMsgCatalog *MyLocale::FindCatalog(const wxChar *szDomain) const
{
    // linear search in the linked list
    MyMsgCatalog *pMsgCat;
    for ( pMsgCat = m_pMsgCat; pMsgCat != NULL; pMsgCat = pMsgCat->m_pNext )
    {
        if ( wxStricmp(pMsgCat->GetName(), szDomain) == 0 )
          return pMsgCat;
    }

    return NULL;
}

// check if the given catalog is loaded
bool MyLocale::IsLoaded(const wxChar *szDomain) const
{
    return FindCatalog(szDomain) != NULL;
}

bool MyLocale::AddCatalogFromMemory(const MyLocaleMemoryCatalog& msgCatalogMemory)
{
    wxString strlocale = GetCanonicalName();
    wxString sublocale = strlocale.BeforeFirst(_T('_'));

    for (unsigned int cati = 0; msgCatalogMemory.langs[cati].msgIdLanguage; ++cati)
    {
	const MyLocaleMemoryCatalogLanguage& catlang = msgCatalogMemory.langs[cati];

	if (catlang.msgIdLanguage == strlocale ||
	    catlang.msgIdLanguage == sublocale)
	{
	    std::auto_ptr<MyMsgCatalog> pMsgCat (new MyMsgCatalog);

	    if ( pMsgCat->Load(msgCatalogMemory.szDomain,
			       catlang.msgCatalogData, catlang.msgCatalogDataLen,
			       catlang.msgIdCharset, true) )
	    {
		// add it to the head of the list so that in GetString it will
		// be searched before the catalogs added earlier
		pMsgCat->m_pNext = m_pMsgCat;
		m_pMsgCat = pMsgCat.release();

		return true;
	    }
	}
    }

    const wxLanguage msgIdLanguage = wxLANGUAGE_ENGLISH_US;

    // It is OK to not load catalog if the msgid language and m_language match,
    // in which case we can directly display the texts embedded in program's
    // source code:
    if (GetLanguage() == msgIdLanguage)
	return true;

    // If there's no exact match, we may still get partial match where the
    // (basic) language is same, but the country differs. For example, it's
    // permitted to use en_US strings from sources even if m_language is en_GB:
    const wxLanguageInfo *msgIdLangInfo = GetLanguageInfo(msgIdLanguage);
    if ( msgIdLangInfo && msgIdLangInfo->CanonicalName.Mid(0, 2) == GetCanonicalName().Mid(0, 2) )
    {
	return true;
    }

    return false;
}
