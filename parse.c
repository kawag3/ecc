#include "ecc.h"

LVar *locals;

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}


// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}



bool peek_type() {
  if (token->kind != TK_INT)
    return false;
  return true;
}



// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if ((token->kind != TK_RESERVED
    && token->kind != TK_RETURN
    && token->kind != TK_IF // !!! TK_KWとしてまとめる手も
    && token->kind != TK_ELSE
    && token->kind != TK_WHILE
    && token->kind != TK_FOR
    && token->kind != TK_INT)
   || token->len != strlen(op)
   || memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

//
Token *consume_ident() {
  Token *tk = token;
  //if (token->kind != TK_IDENT
  // || token->len != 1)
  if (token->kind != TK_IDENT)
    return NULL;
  token = token->next;
  return tk;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if ((token->kind != TK_RESERVED
    && token->kind != TK_RETURN
    && token->kind != TK_IF
    && token->kind != TK_ELSE
    && token->kind != TK_WHILE
    && token->kind != TK_FOR
    && token->kind != TK_INT)
   || token->len != strlen(op)
   || memcmp(token->str, op, token->len))
    //error("'%c'ではありません", op);
    error_at(token->str, "\"%s\"ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    //error("数ではありません");
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// 与えられた文字がトークンを構成する文字か
//int is_alnum(char c) {
int is_aA0_(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

int is_aA_(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         (c == '_');
}

// 入力文字列pをトークナイズしてそれを返す
//Token *tokenize(char *p) {
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }


    if (!memcmp(p, "==", 2) || !memcmp(p, "!=", 2)
     || !memcmp(p, "<=", 2) || !memcmp(p, ">=", 2)) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }


    if (*p == '+' || *p == '-'
     || *p == '<' || *p == '>'
     || *p == '*' || *p == '/' || *p == '(' || *p == ')'
     || *p == '=' || *p == ';'
     || *p == '{' || *p == '}'
     || *p == ',' || *p == '&') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0); // 0 is dummy len
      char *oldp = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - oldp;
      continue;
    }



    //if (!strncmp(p, "return", 6) && !is_alnum(p[6])) {
    if (!strncmp(p, "return", 6) && !is_aA0_(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      cur->len = 6;
      p += 6;
      continue;
    }
    else if (!strncmp(p, "if", 2) && !is_aA0_(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      cur->len = 2;
      p += 2;
      continue;
    }
    else if (!strncmp(p, "else", 4) && !is_aA0_(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      cur->len = 4;
      p += 4;
      continue;
    }
    else if (!strncmp(p, "while", 5) && !is_aA0_(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      cur->len = 5;
      p += 5;
      continue;
    }
    else if (!strncmp(p, "for", 3) && !is_aA0_(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      cur->len = 3;
      p += 3;
      continue;
    }

    else if (!strncmp(p, "int", 3) && !is_aA0_(p[3])) {
      cur = new_token(TK_INT, cur, p, 3);
      cur->len = 3;
      p += 3;
      continue;
    }



    //if ('a' <= *p && *p <= 'z') {
    if (is_aA_(*p)) {
    //  cur = new_token(TK_IDENT, cur, p++, 1);
    //  cur->len = 1;
      char *oldp = p++;
      //while (('a' <= *p && *p <= 'z') || ('0' <= *p && *p <= '9'))
      while (is_aA0_(*p))
        p++;
      cur = new_token(TK_IDENT, cur, oldp, p - oldp);
      cur->len = p - oldp;
      continue;
    }

    //error("トークナイズできません");
    //error_at(token->str, "トークナイズできません");
    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0); // 0 is dummy len
  return head.next;
}



Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}



LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
   if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
     return var;
  return NULL;
}




void program() {
  int i = 0;
  while (!at_eof())
    //code[i++] = stmt();
    code[i++] = function();
  code[i] = NULL;
}



Node *function() {
  Node *nd;

  locals = NULL;

  expect("int");

  Token *tok = consume_ident();
  if (!tok)
    error_at(tok->str, "IDENTではありません");
  expect("(");
  Node *node = calloc(1, sizeof(Node));

  if (!consume(")")) { // 仮引数が1個以上ある
    //expect("int");
    node->args = arg(); // 仮引数1個目
    nd = node->args;
    while (!consume(")")) { // 仮引数2個目以降
      expect(",");
      //expect("int");
      nd->next = arg();
      nd = nd->next;
    }
  }

  // 関数本体はBLOCK
  nd = stmt();
  if (nd->kind != ND_BLOCK)
    //error("BLOCKではありません"); // TODO
    error_at(token->str, "BLOCKではありません"); // TODO
  node->body = nd;

  node->kind = ND_FNDEF;
  node->str = tok->str; // TODO 同名の関数の多重定義チェックが必要なので先に
  node->len = tok->len;

  return node;
}



Node *stmt() {
  Node *node;

  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr(); // lhs only, no rhs??
    expect(";");
  }
  else if (consume("if")) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else"))
      node->els = stmt();
  }
  else if (consume("while")) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->cond = expr();
    expect(")");
    node->body= stmt();
  }
  else if (consume("for")) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = expr();
      expect(")");
    }
    node->body = stmt();
  }
  else if (consume("{")) {
    Node head = {};
    Node *cur = &head;

    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }

    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->body = head.next;
  }
  else if (consume(";")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_EMPTYSTMT;
  }


  else if (peek_type()) { // TODO stmt()の中でやるのは適切か？
    node = localvar();
    expect(";");
  }


  else {
    Node *nd = expr();
    expect(";");

    node = calloc(1, sizeof(Node));
    node->kind = ND_EXPRSTMT;
    node->lhs = nd;
  }

  return node;
}

Node *expr() {
  //return assign();
  return comma();
}


Node *comma() {
  Node *node = assign();

  for (;;) {
    if (consume(","))
      node = new_node(ND_COMMA, node, assign());
    else
      return node;
  }

}

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign()); // !!!
    //node = new_node(ND_ASSIGN, node, equality());

  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      //node = new_node(ND_GT, node, add());
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      //node = new_node(ND_GE, node, add());
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}


Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume("+"))
    return primary();
  else if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  else if (consume("&"))
    return new_node(ND_REF, NULL, unary()); // BNF!!!
  else if (consume("*"))
    return new_node(ND_DEREF, NULL, unary()); // BNF!!!
  else
    return primary();
}

Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }


  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));

    //if (consume("(")) {
    //  expect(")");


    if (consume("(")) { // 関数呼び出しである
      if (!consume(")")) { // 実引数が1個以上ある
        //node->args = expr(); //←実引数区切りの,をコンマ演算子と解釈してしまう
        node->args = assign(); // 実引数1個目
        Node *nd = node->args;
        while (!consume(")")) { // 実引数2個目以降
          expect(",");
          //nd->next = expr(); //←実引数区切りの,をコンマ演算子と解釈してしまう
          nd->next = assign();
          nd = nd->next;
        }
      }

      node->kind = ND_FNCALL;
      node->str = tok->str;
      node->len = tok->len;

      return node;
    }

    node->kind = ND_LVAR;

    //node->offset = (tok->str[0] - 'a' + 1) * 8;
    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    }
    else {
      error_at(tok->str, "未定義です");
/*
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if (locals)
        lvar->offset = locals->offset + 8; // core dump !!!
      else
        lvar->offset = 8;
      node->offset = lvar->offset;
      locals = lvar;
*/
    }

    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}

Node *ident() {
  Type *ty = calloc(1, sizeof(Type));
  if(consume("int"))
    ty->kind = TY_INT;
  else
    error_at(token->str, "型ではありません");
  while (consume("*"))
    ty->refdepth++;

  Token *tok = consume_ident();
  if (!tok)
    error_at(tok->str, "IDENTではありません");

  Node *node = calloc(1, sizeof(Node));
  //node->kind = ND_ARG; // localvar()とarg()の違いはここだけ
  node->type = ty;

  LVar *lvar = find_lvar(tok);
  if (lvar) {
    //node->offset = lvar->offset;
    error_at(tok->str, "二重定義です");
  }
  else {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (locals)
      lvar->offset = locals->offset + 8; // core dump !!!
    else
      lvar->offset = 8;
    node->offset = lvar->offset;
    locals = lvar;
  }

  return node;
}

Node *arg() {
  Node *node = ident();
  node->kind = ND_ARG; // localvar()とarg()の違いはここだけ

  return node;
}

Node *localvar() {
  Node *node = ident();
  node->kind = ND_LVDEF; // localvar()とarg()の違いはここだけ

  return node;
}

