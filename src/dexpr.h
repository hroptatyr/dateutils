/*** dexpr.h -- helper for date expressions */
#if !defined INCLUDED_dexpr_h_
#define INCLUDED_dexpr_h_

typedef struct dexpr_s *dexpr_t;
typedef const struct dexpr_s *const_dexpr_t;

typedef enum {
	DEX_UNK,
	DEX_VAL,
	DEX_CONJ,
	DEX_DISJ,
} dex_type_t;

struct dexpr_s {
	dex_type_t type:31;
	unsigned int nega:1;
	dexpr_t left;
	union {
		void *value;
		dexpr_t right;
	};
};

/* parser routine */
extern int dexpr_parse(dexpr_t *root, char *s, size_t l);

extern int yyparse(dexpr_t *cur);

#define YYSTYPE		dexpr_t

#endif	/* INCLUDED_dexpr_h_ */
