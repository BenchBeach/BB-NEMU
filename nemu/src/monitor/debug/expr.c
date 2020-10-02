#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 0, PLUS, MINUS, STAR, DIV,                                // this write style make each ++, broad my horizon!!!
	EQ, NOTEQ, OR, AND,
 	NOT, NEG, POINTER,
	LB, RB, HEX, DEC, REG,

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				//spaces
	{"\\+", PLUS},					//plus
	{"-", MINUS},					//minus
	{"\\*", STAR},					//star
	{"/", DIV},						//div
	{"==", EQ},						//eq
	{"!=", NOTEQ},					//noteq
	{"&&", AND},					//and
	{"\\|\\|", OR},					//or
	{"!", NOT},						//not
	{"\\(", LB},					//lb
	{"\\)", RB},					//rb
	{"0[xX][0-9a-zA-Z]+", HEX},		//hex
	{"[0-9]+", DEC},				//dec
	{"\\$[a-z]+", REG}				//reg
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				// char *substr_start = e + position;
				 int substr_len = pmatch.rm_eo;
				// Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE:
						break;											//It's blank!
					case HEX:case DEC:case REG:
						strncpy(tokens[nr_token].str, e + position - substr_len, substr_len);//regs or number
						tokens[nr_token].str[substr_len] = '\0';		//add '\0', it's very important
						//WARNING: 64 may be a little small...
					default:
						if(rules[i].token_type == MINUS) {	//solve neg
							if(nr_token == 0) tokens[nr_token++].type = NEG;
							else if(PLUS <= tokens[nr_token - 1].type && tokens[nr_token - 1].type <= LB) {
								tokens[nr_token++].type = NEG;
							} else tokens[nr_token++].type = MINUS;
						} else if(rules[i].token_type == STAR) { //solve pointer
							if(nr_token == 0) tokens[nr_token++].type = POINTER;
							else if(PLUS <= tokens[nr_token - 1].type && tokens[nr_token - 1].type <= LB) {
								tokens[nr_token++].type = POINTER;
							} else tokens[nr_token++].type = STAR;
						} else {
							tokens[nr_token++].type = rules[i].token_type;	//other	
						}
						break;
					//panic("please implement me");
				}
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
	// I am wander what license should i take if i push it to github?
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}

