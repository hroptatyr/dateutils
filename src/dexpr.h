/*** dexpr.h -- helper for date expressions */
#if !defined INCLUDED_dexpr_h_
#define INCLUDED_dexpr_h_

#include "date-core.h"

typedef struct dexpr_s *dexpr_t;
typedef const struct dexpr_s *const_dexpr_t;

typedef struct dexkv_s *dexkv_t;
typedef const struct dexkv_s *const_dexkv_t;

typedef enum {
	DEX_UNK,
	DEX_VAL,
	DEX_CONJ,
	/* must be last as other types will be considered inferior */
	DEX_DISJ,
} dex_type_t;

struct dexkv_s {
	struct dt_spec_s sp;
	oper_t op:3;
	union {
		struct dt_d_s d;
		signed int s;
		const char *dstr;
	};
};

struct dexpr_s {
	dex_type_t type:31;
	unsigned int nega:1;
	dexpr_t left;
	union {
		struct dexkv_s kv[1];
		struct {
			dexpr_t right;
			dexpr_t up;
		};
	};
};


/* parser routine */
extern int dexpr_parse(dexpr_t *root, char *s, size_t l);

#endif	/* INCLUDED_dexpr_h_ */
