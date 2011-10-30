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
	DEX_NEGA,
} dex_type_t;

struct dexpr_s {
	dex_type_t type;
	void *value;
	dexpr_t left;
	dexpr_t right;
};

#endif	/* INCLUDED_dexpr_h_ */
