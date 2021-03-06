#ifndef SMAP_H
#define SMAP_H
#ifdef __cplusplus
extern "C" {
#endif

/*
 * smap.h
 *
 * #SHORTDOC map handling (key-value storage)
 *
 * #DOC Map functions handle key-value storage, which is implemented as a
 * #DOC Red-Black tree (O(log n) time complexity for insert/read/delete)
 * #DOC
 * #DOC
 * #DOC Supported key/value modes (enum eSM_Type):
 * #DOC
 * #DOC
 * #DOC	SM_II32: 32-bit integer key, 32-bit integer value
 * #DOC
 * #DOC	SM_UU32: 32-bit unsigned int key, 32-bit unsigned int value
 * #DOC
 * #DOC	SM_II: 64-bit int key, 64-bit int value
 * #DOC
 * #DOC	SM_IS: 64-bit int key, string value
 * #DOC
 * #DOC	SM_IP: 64-bit int key, pointer value
 * #DOC
 * #DOC	SM_SI: 64-bit string key, 64-bit int value
 * #DOC
 * #DOC	SM_SS: string key, string value
 * #DOC
 * #DOC	SM_SP: string key, pointer value
 * #DOC
 * #DOC
 * #DOC Callback types for the sm_itr_*() functions:
 * #DOC
 * #DOC
 * #DOC	typedef srt_bool (*srt_map_it_ii32)(int32_t k, int32_t v, void *context);
 * #DOC
 * #DOC	typedef srt_bool (*srt_map_it_uu32)(uint32_t k, uint32_t v, void *context);
 * #DOC
 * #DOC	typedef srt_bool (*srt_map_it_ii)(int64_t k, int64_t v, void *context);
 * #DOC
 * #DOC	typedef srt_bool (*srt_map_it_is)(int64_t k, const srt_string *, void *context);
 * #DOC
 * #DOC	typedef srt_bool (*srt_map_it_ip)(int64_t k, const void *, void *context);
 * #DOC
 * #DOC	typedef srt_bool (*srt_map_it_si)(const srt_string *, int64_t v, void *context);
 * #DOC
 * #DOC	typedef srt_bool (*srt_map_it_ss)(const srt_string *, const srt_string *, void *context);
 * #DOC
 * #DOC	typedef srt_bool (*srt_map_it_sp)(const srt_string *, const void *, void *context);
 *
 * Copyright (c) 2015-2018 F. Aragon. All rights reserved.
 * Released under the BSD 3-Clause License (see the doc/LICENSE)
 */

#include "saux/stree.h"
#include "sstring.h"
#include "svector.h"

/*
 * Structures
 */

enum eSM_Type0 {
	SM0_II32,
	SM0_UU32,
	SM0_II,
	SM0_IS,
	SM0_IP,
	SM0_SI,
	SM0_SS,
	SM0_SP,
	SM0_I,
	SM0_I32,
	SM0_U32,
	SM0_S
};

enum eSM_Type {
	SM_II32 = SM0_II32,
	SM_UU32 = SM0_UU32,
	SM_II = SM0_II,
	SM_IS = SM0_IS,
	SM_IP = SM0_IP,
	SM_SI = SM0_SI,
	SM_SS = SM0_SS,
	SM_SP = SM0_SP
};

#ifndef S_DISABLE_SM_STRING_OPTIMIZATION
#define S_ENABLE_SM_STRING_OPTIMIZATION
#endif

#ifdef S_ENABLE_SM_STRING_OPTIMIZATION
#define SMStrRaw 24
#define SMStrAllocSize (SMStrRaw - 1)
#define SMStrMaxSize (SMStrAllocSize - 5)
#define SMStr_Null 0
#define SMStr_Direct 1
#define SMStr_Indirect 2
struct SMStrD {
	uint8_t t;
	uint8_t s_raw[SMStrAllocSize];
};
struct SMStrI {
	uint8_t t;
	srt_string *s;
};
union SMStr {
	uint8_t t;
	struct SMStrD d;
	struct SMStrI i;
};
#else
union SMStr {
	srt_string *s;
};
#endif

struct SMapI {
	srt_tnode n;
	int64_t k;
};
struct SMapS {
	srt_tnode n;
	union SMStr k;
};
struct SMapi {
	srt_tnode n;
	int32_t k;
};
struct SMapu {
	srt_tnode n;
	uint32_t k;
};
struct SMapii {
	struct SMapi x;
	int32_t v;
};
struct SMapuu {
	struct SMapu x;
	uint32_t v;
};
struct SMapII {
	struct SMapI x;
	int64_t v;
};
struct SMapIS {
	struct SMapI x;
	union SMStr v;
};
struct SMapIP {
	struct SMapI x;
	const void *v;
};
struct SMapSI {
	struct SMapS x;
	int64_t v;
};
struct SMapSS {
	struct SMapS x;
	union SMStr v;
};
struct SMapSP {
	struct SMapS x;
	const void *v;
};

typedef srt_tree srt_map; /* Opaque structure (accessors are provided) */
			  /* (map is implemented as a tree)	     */

typedef srt_bool (*srt_map_it_ii32)(int32_t k, int32_t v, void *context);
typedef srt_bool (*srt_map_it_uu32)(uint32_t k, uint32_t v, void *context);
typedef srt_bool (*srt_map_it_ii)(int64_t k, int64_t v, void *context);
typedef srt_bool (*srt_map_it_is)(int64_t k, const srt_string *, void *context);
typedef srt_bool (*srt_map_it_ip)(int64_t k, const void *, void *context);
typedef srt_bool (*srt_map_it_si)(const srt_string *, int64_t v, void *context);
typedef srt_bool (*srt_map_it_ss)(const srt_string *, const srt_string *, void *context);
typedef srt_bool (*srt_map_it_sp)(const srt_string *, const void *, void *context);

/*
 * Allocation
 */

/*
#API: |Allocate map (stack)|map type; initial reserve|map|O(1)|1;2|
srt_map *sm_alloca(const enum eSM_Type t, const size_t n);
*/
#define sm_alloca(type, max_size)                                              \
	sm_alloc_raw(type, S_TRUE,                                             \
		     s_alloca(sd_alloc_size_raw(sizeof(srt_map),               \
						sm_elem_size(type), max_size,  \
						S_FALSE)),                     \
		     sm_elem_size(type), max_size)

srt_map *sm_alloc_raw0(const enum eSM_Type0 t, const srt_bool ext_buf,
		       void *buffer, const size_t elem_size,
		       const size_t max_size);

S_INLINE srt_map *sm_alloc_raw(const enum eSM_Type t, const srt_bool ext_buf,
			       void *buffer, const size_t elem_size,
			       const size_t max_size)
{
	return sm_alloc_raw0((enum eSM_Type0)t, ext_buf, buffer, elem_size,
			     max_size);
}

srt_map *sm_alloc0(const enum eSM_Type0 t,
		   const size_t initial_num_elems_reserve);

/* #API: |Allocate map (heap)|map type; initial reserve|map|O(1)|1;2| */
S_INLINE srt_map *sm_alloc(const enum eSM_Type t, const size_t initial_num_elems_reserve)
{
	return sm_alloc0((enum eSM_Type0)t, initial_num_elems_reserve);
}

/* #API: |Get map node size from map type|map type|bytes required for storing a single node|O(1)|1;2| */
S_INLINE uint8_t sm_elem_size(const int t)
{
	switch (t) {
	case SM0_II32:
		return sizeof(struct SMapii);
	case SM0_UU32:
		return sizeof(struct SMapuu);
	case SM0_II:
		return sizeof(struct SMapII);
	case SM0_IS:
		return sizeof(struct SMapIS);
	case SM0_IP:
		return sizeof(struct SMapIP);
	case SM0_SI:
		return sizeof(struct SMapSI);
	case SM0_SS:
		return sizeof(struct SMapSS);
	case SM0_SP:
		return sizeof(struct SMapSP);
	case SM0_I32:
		return sizeof(struct SMapi);
	case SM0_U32:
		return sizeof(struct SMapu);
	case SM0_I:
		return sizeof(struct SMapI);
	case SM0_S:
		return sizeof(struct SMapS);
	default:
		break;
	}
	return 0;
}

/* #API: |Duplicate map|input map|output map|O(n)|1;2| */
srt_map *sm_dup(const srt_map *src);

/* #API: |Reset/clean map (keeping map type)|map|-|O(1) for simple maps, O(n) for maps having nodes with strings|1;2| */
void sm_clear(srt_map *m);

/*
#API: |Free one or more maps (heap)|map; more maps (optional)|-|O(1) for simple maps, O(n) for maps having nodes with strings|1;2|
void sm_free(srt_map **m, ...)
*/
#ifdef S_USE_VA_ARGS
#define sm_free(...) sm_free_aux(__VA_ARGS__, S_INVALID_PTR_VARG_TAIL)
#else
#define sm_free(m) sm_free_aux(m, S_INVALID_PTR_VARG_TAIL)
#endif
void sm_free_aux(srt_map **m, ...);

SD_BUILDFUNCS_FULL_ST(sm, srt_map, 0)

/*
#API: |Ensure space for extra elements|map;number of extra elements|extra size allocated|O(1)|1;2|
size_t sm_grow(srt_map **m, const size_t extra_elems)

#API: |Ensure space for elements|map;absolute element reserve|reserved elements|O(1)|1;2|
size_t sm_reserve(srt_map **m, const size_t max_elems)

#API: |Make the map use the minimum possible memory|map|map reference (optional usage)|O(1) for allocators using memory remap; O(n) for naive allocators|1;2|
srt_map *sm_shrink(srt_map **m);

#API: |Get map size|map|Map number of elements|O(1)|1;2|
size_t sm_size(const srt_map *m);

#API: |Allocated space|map|current allocated space (vector elements)|O(1)|1;2|
size_t sm_capacity(const srt_map *m);

#API: |Preallocated space left|map|allocated space left|O(1)|1;2|
size_t sm_capacity_left(const srt_map *m);

#API: |Tells if a map is empty (zero elements)|map|S_TRUE: empty vector; S_FALSE: not empty|O(1)|1;2|
srt_bool sm_empty(const srt_map *m)
*/

/*
 * Copy
 */

/* #API: |Overwrite map with a map copy|output map; input map|output map reference (optional usage)|O(n)|1;2| */
srt_map *sm_cpy(srt_map **m, const srt_map *src);

/*
 * Random access
 */

/* #API: |Access to int32-int32 map|map; int32 key|int32|O(log n)|1;2| */
int32_t sm_at_ii32(const srt_map *m, const int32_t k);

/* #API: |Access to uint32-uint32 map|map; uint32 key|uint32|O(log n)|1;2| */
uint32_t sm_at_uu32(const srt_map *m, const uint32_t k);

/* #API: |Access to integer-interger map|map; integer key|integer|O(log n)|1;2| */
int64_t sm_at_ii(const srt_map *m, const int64_t k);

/* #API: |Access to integer-string map|map; integer key|string|O(log n)|1;2| */
const srt_string *sm_at_is(const srt_map *m, const int64_t k);

/* #API: |Access to integer-pointer map|map; integer key|pointer|O(log n)|1;2| */
const void *sm_at_ip(const srt_map *m, const int64_t k);

/* #API: |Access to string-integer map|map; string key|integer|O(log n)|1;2| */
int64_t sm_at_si(const srt_map *m, const srt_string *k);

/* #API: |Access to string-string map|map; string key|string|O(log n)|1;2| */
const srt_string *sm_at_ss(const srt_map *m, const srt_string *k);

/* #API: |Access to string-pointer map|map; string key|pointer|O(log n)|1;2| */
const void *sm_at_sp(const srt_map *m, const srt_string *k);

/*
 * Existence check
 */

/* #API: |Map element count/check|map; 32-bit unsigned integer key|S_TRUE: element found; S_FALSE: not in the map|O(log n)|1;2| */
srt_bool sm_count_u(const srt_map *m, const uint32_t k);

/* #API: |Map element count/check|map; integer key|S_TRUE: element found; S_FALSE: not in the map|O(log n)|1;2| */
srt_bool sm_count_i(const srt_map *m, const int64_t k);

/* #API: |Map element count/check|map; string key|S_TRUE: element found; S_FALSE: not in the map|O(log n)|1;2| */
srt_bool sm_count_s(const srt_map *m, const srt_string *k);

/*
 * Insert
 */

/* #API: |Insert into int32-int32 map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_insert_ii32(srt_map **m, const int32_t k, const int32_t v);

/* #API: |Insert into uint32-uint32 map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_insert_uu32(srt_map **m, const uint32_t k, const uint32_t v);

/* #API: |Insert into int-int map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_insert_ii(srt_map **m, const int64_t k, const int64_t v);

/* #API: |Insert into int-string map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_insert_is(srt_map **m, const int64_t k, const srt_string *v);

/* #API: |Insert into int-pointer map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_insert_ip(srt_map **m, const int64_t k, const void *v);

/* #API: |Insert into string-int map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_insert_si(srt_map **m, const srt_string *k, const int64_t v);

/* #API: |Insert into string-string map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_insert_ss(srt_map **m, const srt_string *k, const srt_string *v);

/* #API: |Insert into string-pointer map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_insert_sp(srt_map **m, const srt_string *k, const void *v);

/* #API: |Increment value into int32-int32 map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_inc_ii32(srt_map **m, const int32_t k, const int32_t v);

/* #API: |Increment into uint32-uint32 map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_inc_uu32(srt_map **m, const uint32_t k, const uint32_t v);

/* #API: |Increment into int-int map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_inc_ii(srt_map **m, const int64_t k, const int64_t v);

/* #API: |Increment into string-int map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|1;2| */
srt_bool sm_inc_si(srt_map **m, const srt_string *k, const int64_t v);

/*
 * Delete
 */

/* #API: |Delete map element|map; integer key|S_TRUE: found and deleted; S_FALSE: not found|O(log n)|1;2| */
srt_bool sm_delete_i(srt_map *m, const int64_t k);

/* #API: |Delete map element|map; string key|S_TRUE: found and deleted; S_FALSE: not found|O(log n)|1;2| */
srt_bool sm_delete_s(srt_map *m, const srt_string *k);

/*
 * Enumeration / export data
 */

/* #API: |Enumerate map elements in a given key range|map; key lower bound; key upper bound; callback function; callback function context|Elements processed|O(log n) + O(log m); additional 2 * O(log n) space required, allocated on the stack, i.e. fast|1;2| */
size_t sm_itr_ii32(const srt_map *m, int32_t key_min, int32_t key_max, srt_map_it_ii32 f, void *context);

/* #API: |Enumerate map elements in a given key range|map; key lower bound; key upper bound; callback function; callback function context|Elements processed|O(log n) + O(log m); additional 2 * O(log n) space required, allocated on the stack, i.e. fast|1;2| */
size_t sm_itr_uu32(const srt_map *m, uint32_t key_min, uint32_t key_max, srt_map_it_uu32 f, void *context);

/* #API: |Enumerate map elements in a given key range|map; key lower bound; key upper bound; callback function; callback function context|Elements processed|O(log n) + O(log m); additional 2 * O(log n) space required, allocated on the stack, i.e. fast|1;2| */
size_t sm_itr_ii(const srt_map *m, int64_t key_min, int64_t key_max, srt_map_it_ii f, void *context);

/* #API: |Enumerate map elements in a given key range|map; key lower bound; key upper bound; callback function; callback function context|Elements processed|O(log n) + O(log m); additional 2 * O(log n) space required, allocated on the stack, i.e. fast|1;2| */
size_t sm_itr_is(const srt_map *m, int64_t key_min, int64_t key_max, srt_map_it_is f, void *context);

/* #API: |Enumerate map elements in a given key range|map; key lower bound; key upper bound; callback function; callback function context|Elements processed|O(log n) + O(log m); additional 2 * O(log n) space required, allocated on the stack, i.e. fast|1;2| */
size_t sm_itr_ip(const srt_map *m, int64_t key_min, int64_t key_max, srt_map_it_ip f, void *context);

/* #API: |Enumerate map elements in a given key range|map; key lower bound; key upper bound; callback function; callback function context|Elements processed|O(log n) + O(log m); additional 2 * O(log n) space required, allocated on the stack, i.e. fast|1;2| */
size_t sm_itr_si(const srt_map *m, const srt_string *key_min, const srt_string *key_max, srt_map_it_si f, void *context);

/* #API: |Enumerate map elements in a given key range|map; key lower bound; key upper bound; callback function; callback function context|Elements processed|O(log n) + O(log m); additional 2 * O(log n) space required, allocated on the stack, i.e. fast|1;2| */
size_t sm_itr_ss(const srt_map *m, const srt_string *key_min, const srt_string *key_max, srt_map_it_ss f, void *context);

/* #API: |Enumerate map elements in a given key range|map; key lower bound; key upper bound; callback function; callback function context|Elements processed|O(log n) + O(log m); additional 2 * O(log n) space required, allocated on the stack, i.e. fast|1;2| */
size_t sm_itr_sp(const srt_map *m, const srt_string *key_min, const srt_string *key_max, srt_map_it_sp f, void *context);

/* #NOTAPI: |Sort map to vector (used for test coverage, not as documented API)|map; output vector for keys; output vector for values|Number of map elements|O(n)|1;2| */
ssize_t sm_sort_to_vectors(const srt_map *m, srt_vector **kv, srt_vector **vv);

/*
 * Auxiliary inlined functions
 */

#ifdef S_ENABLE_SM_STRING_OPTIMIZATION

S_INLINE const srt_string *SMStrGet(const union SMStr *s)
{
	return !s || s->t == SMStr_Null
		       ? ss_void
		       : s->t == SMStr_Direct ? (const srt_string *)s->d.s_raw
					      : s->i.s;
}

void SMStrUpdate_unsafe(union SMStr *sstr, const srt_string *s);

S_INLINE void SMStrUpdate(union SMStr *sstr, const srt_string *s)
{
	if (sstr)
		SMStrUpdate_unsafe(sstr, s);
}

S_INLINE void SMStrSet(union SMStr *sstr, const srt_string *s)
{
	if (sstr) {
		sstr->t = SMStr_Null;
		SMStrUpdate(sstr, s);
	}
}

S_INLINE void SMStrSetRef(union SMStr *sstr, const srt_string *s)
{
	if (sstr) {
		sstr->t = SMStr_Indirect;
		sstr->i.s = (srt_string *)s; /* CONSTNESS */
	}
}

S_INLINE void SMStrFree(union SMStr *sstr)
{
	if (sstr && sstr->t == SMStr_Indirect)
		ss_free(&sstr->i.s);
}

#else

S_INLINE const srt_string *SMStrGet(const union SMStr *s)
{
	return s ? s->s : ss_void;
}

S_INLINE void SMStrUpdate(union SMStr *sstr, const srt_string *s)
{
	if (sstr)
		ss_cpy(&sstr->s, s);
}

S_INLINE void SMStrSet(union SMStr *sstr, const srt_string *s)
{
	if (sstr)
		sstr->s = ss_dup(s);
}

S_INLINE void SMStrSetRef(union SMStr *sstr, const srt_string *s)
{
	if (sstr)
		sstr->s = (srt_string *)s; /* CONSTNESS */
}

S_INLINE void SMStrFree(union SMStr *sstr)
{
	if (sstr)
		ss_free(&sstr->s);
}

#endif /* #ifdef S_ENABLE_SM_STRING_OPTIMIZATION */

	/*
	 * Unordered enumeration is inlined in order to get almost as fast
	 * as array access after compiler optimization.
	 */

#define S_SM_ENUM_AUX_K(NT, m, i, n_k, def_k)                                  \
	const NT *n = (const NT *)st_enum_r(m, i);                             \
	RETURN_IF(!n, def_k);                                                  \
	return n_k

#define S_SM_ENUM_AUX_V(t, NT, m, i, n_v, def_v)                               \
	const NT *n;                                                           \
	RETURN_IF(!m || t != m->d.sub_type, def_v);                            \
	n = (const NT *)st_enum_r(m, i);                                       \
	RETURN_IF(!n, def_v);                                                  \
	return n_v

/* #API: |Enumerate int32-* map keys|map; element, 0 to n - 1|int32_t|O(1)|1;2| */
S_INLINE int32_t sm_it_i32_k(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_K(struct SMapi, m, i, n->k, 0);
}

/* #API: |Enumerate int32-int32 map values|map; element, 0 to n - 1|int32_t|O(1)|1;2| */
S_INLINE int32_t sm_it_ii32_v(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_V(SM_II32, struct SMapii, m, i, n->v, 0);
}

/* #API: |Enumerate uint32-* map keys|map; element, 0 to n - 1|uint32_t|O(1)|1;2| */
S_INLINE uint32_t sm_it_u32_k(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_K(struct SMapu, m, i, n->k, 0);
}

/* #API: |Enumerate uint32-uint32 map values|map; element, 0 to n - 1|uint32_t|O(1)|1;2| */
S_INLINE uint32_t sm_it_uu32_v(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_V(SM_UU32, struct SMapuu, m, i, n->v, 0);
}

/* #API: |Enumerate integer-* map keys|map; element, 0 to n - 1|int64_t|O(1)|1;2| */
S_INLINE int64_t sm_it_i_k(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_K(struct SMapI, m, i, n->k, 0);
}

/* #API: |Enumerate integer-interger map values|map; element, 0 to n - 1|int64_t|O(1)|1;2| */
S_INLINE int64_t sm_it_ii_v(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_V(SM_II, struct SMapII, m, i, n->v, 0);
}

/* #API: |Enumerate integer-string map values|map; element, 0 to n - 1|string|O(1)|1;2| */
S_INLINE const srt_string *sm_it_is_v(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_V(SM_IS, struct SMapIS, m, i, SMStrGet(&n->v), ss_void);
}

/* #API: |Enumerate integer-pointer map values|map; element, 0 to n - 1|pointer|O(1)|1;2| */
S_INLINE const void *sm_it_ip_v(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_V(SM_IP, struct SMapIP, m, i, n->v, NULL);
}

/* #API: |Enumerate string-* map keys|map; element, 0 to n - 1|string|O(1)|1;2| */
S_INLINE const srt_string *sm_it_s_k(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_K(struct SMapS, m, i, SMStrGet(&n->k), ss_void);
}

/* #API: |Enumerate string-integer map values|map; element, 0 to n - 1|int64_t|O(1)|1;2| */
S_INLINE int64_t sm_it_si_v(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_V(SM_SI, struct SMapSI, m, i, n->v, 0);
}

/* #API: |Enumerate string-string map values|map; element, 0 to n - 1|string|O(1)|1;2| */
S_INLINE const srt_string *sm_it_ss_v(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_V(SM_SS, struct SMapSS, m, i, SMStrGet(&n->v), ss_void);
}

/* #API: |Enumerate string-pointer map|map; element, 0 to n - 1|pointer|O(1)|1;2| */
S_INLINE const void *sm_it_sp_v(const srt_map *m, const srt_tndx i)
{
	S_SM_ENUM_AUX_V(SM_SP, struct SMapSP, m, i, n->v, NULL);
}

	/*
	 * Templates (internal usage)
	 */

#define SM_ENUM_INORDER_XX(FN, CALLBACK_T, MAP_TYPE, KEY_T, TR_CMP_MIN,        \
			   TR_CMP_MAX, TR_CALLBACK)                            \
	size_t FN(const srt_map *m, KEY_T kmin, KEY_T kmax, CALLBACK_T f,      \
		  void *context)                                               \
	{                                                                      \
		ssize_t level;                                                 \
		size_t ts, nelems, rbt_max_depth;                              \
		struct STreeScan *p;                                           \
		const srt_tnode *cn;                                           \
		int cmpmin, cmpmax;                                            \
		RETURN_IF(!m, 0);			 /* null tree */       \
		RETURN_IF(m->d.sub_type != MAP_TYPE, 0); /* wrong type */      \
		ts = sm_size(m);                                               \
		RETURN_IF(!ts, S_FALSE); /* empty tree */                      \
		level = 0;                                                     \
		nelems = 0;                                                    \
		rbt_max_depth = 2 * (slog2(ts) + 1);                           \
		p = (struct STreeScan *)s_alloca(sizeof(struct STreeScan)      \
						 * (rbt_max_depth + 3));       \
		ASSERT_RETURN_IF(!p, 0); /* BEHAVIOR: stack error */           \
		p[0].p = ST_NIL;                                               \
		p[0].c = m->root;                                              \
		p[0].s = STS_ScanStart;                                        \
		while (level >= 0) {                                           \
			S_ASSERT(level <= (ssize_t)rbt_max_depth);             \
			switch (p[level].s) {                                  \
			case STS_ScanStart:                                    \
				cn = get_node_r(m, p[level].c);                \
				cmpmin = TR_CMP_MIN;                           \
				cmpmax = TR_CMP_MAX;                           \
				if (cn->x.l != ST_NIL && cmpmin > 0) {         \
					p[level].s = STS_ScanLeft;             \
					level++;                               \
					cn = get_node_r(m, p[level - 1].c);    \
					p[level].c = cn->x.l;                  \
				} else {                                       \
					/* node with null left children */     \
					if (cmpmin >= 0 && cmpmax <= 0) {      \
						if (f && !TR_CALLBACK)         \
							return nelems;         \
						nelems++;                      \
					}                                      \
					if (cn->r != ST_NIL && cmpmax < 0) {   \
						p[level].s = STS_ScanRight;    \
						level++;                       \
						cn = get_node_r(               \
							m, p[level - 1].c);    \
						p[level].c = cn->r;            \
					} else {                               \
						p[level].s = STS_ScanDone;     \
						level--;                       \
						continue;                      \
					}                                      \
				}                                              \
				p[level].p = p[level - 1].c;                   \
				p[level].s = STS_ScanStart;                    \
				continue;                                      \
			case STS_ScanLeft:                                     \
				cn = get_node_r(m, p[level].c);                \
				cmpmin = TR_CMP_MIN;                           \
				cmpmax = TR_CMP_MAX;                           \
				if (cmpmin >= 0 && cmpmax <= 0) {              \
					if (f && !TR_CALLBACK)                 \
						return nelems;                 \
					nelems++;                              \
				}                                              \
				if (cn->r != ST_NIL && cmpmax < 0) {           \
					p[level].s = STS_ScanRight;            \
					level++;                               \
					p[level].p = p[level - 1].c;           \
					cn = get_node_r(m, p[level - 1].c);    \
					p[level].c = cn->r;                    \
					p[level].s = STS_ScanStart;            \
				} else {                                       \
					p[level].s = STS_ScanDone;             \
					level--;                               \
					continue;                              \
				}                                              \
				continue;                                      \
			case STS_ScanRight:                                    \
			case STS_ScanDone:                                     \
				p[level].s = STS_ScanDone;                     \
				level--;                                       \
				continue;                                      \
			}                                                      \
		}                                                              \
		return nelems;                                                 \
	}

S_INLINE int cmp_ni_i(const struct SMapi *a, int32_t b)
{
	return a->k > b ? 1 : a->k < b ? -1 : 0;
}

S_INLINE int cmp_nu_u(const struct SMapu *a, uint32_t b)
{
	return a->k > b ? 1 : a->k < b ? -1 : 0;
}

S_INLINE int cmp_nI_I(const struct SMapI *a, int64_t b)
{
	return a->k > b ? 1 : a->k < b ? -1 : 0;
}

S_INLINE int cmp_ns_s(const struct SMapS *a, const srt_string *b)
{
	return ss_cmp(SMStrGet(&a->k), b);
}

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* #ifndef SMAP_H */
