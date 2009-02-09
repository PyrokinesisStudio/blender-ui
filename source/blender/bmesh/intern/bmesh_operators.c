#include "MEM_guardedalloc.h"

#include "BLI_memarena.h"
#include "BLI_mempool.h"

#include "BKE_utildefines.h"

#include "bmesh.h"
#include "bmesh_private.h"

#include <string.h>

/*forward declarations*/
static void alloc_flag_layer(BMesh *bm);
static void free_flag_layer(BMesh *bm);
static void clear_flag_layer(BMesh *bm);

typedef void (*opexec)(struct BMesh *bm, struct BMOperator *op);

/*mappings map elements to data, which
  follows the mapping struct in memory.*/
typedef struct element_mapping {
	BMHeader *element;
	int len;
} element_mapping;


/*operator slot type information - size of one element of the type given.*/
const int BMOP_OPSLOT_TYPEINFO[] = {
	sizeof(int),
	sizeof(float),
	sizeof(void*),
	0, /* unused */
	0, /* unused */
	0, /* unused */
	sizeof(void*),	/* pointer buffer */
	sizeof(element_mapping)
};

/*
 * BMESH OPSTACK PUSH
 *
 * Pushes the opstack down one level 
 * and allocates a new flag layer if
 * appropriate.
 *
*/

void BMO_push(BMesh *bm, BMOperator *op)
{
	bm->stackdepth++;

	/*add flag layer, if appropriate*/
	if (bm->stackdepth > 1)
		alloc_flag_layer(bm);
	else
		clear_flag_layer(bm);
}

/*
 * BMESH OPSTACK POP
 *
 * Pops the opstack one level  
 * and frees a flag layer if appropriate
 * TODO: investigate NOT freeing flag
 * layers.
 *
*/
void BMO_pop(BMesh *bm)
{
	bm->stackdepth--;
	if(bm->stackdepth > 1)
		free_flag_layer(bm);
}

/*
 * BMESH OPSTACK INIT OP
 *
 * Initializes an operator structure  
 * to a certain type
 *
*/

void BMO_Init_Op(BMOperator *op, int opcode)
{
	int i;

	memset(op, 0, sizeof(BMOperator));
	op->type = opcode;
	
	/*initialize the operator slot types*/
	for(i = 0; i < opdefines[opcode]->totslot; i++) {
		op->slots[i].slottype = opdefines[opcode]->slottypes[i];
		op->slots[i].index = i;
	}

	/*callback*/
	op->exec = opdefines[opcode]->exec;

	/*memarena, used for operator's slot buffers*/
	op->arena = BLI_memarena_new(BLI_MEMARENA_STD_BUFSIZE);
	BLI_memarena_use_calloc (op->arena);
}

/*
 *	BMESH OPSTACK EXEC OP
 *
 *  Executes a passed in operator. This handles
 *  the allocation and freeing of temporary flag
 *  layers and starting/stopping the modelling
 *  loop. Can be called from other operators
 *  exec callbacks as well.
 * 
*/

void BMO_Exec_Op(BMesh *bm, BMOperator *op)
{
	
	BMO_push(bm, op);

	if(bm->stackdepth == 1)
		bmesh_begin_edit(bm);
	op->exec(bm, op);
	
	if(bm->stackdepth == 1)
		bmesh_end_edit(bm,0);
	
	BMO_pop(bm);	
}

/*
 *  BMESH OPSTACK FINISH OP
 *
 *  Does housekeeping chores related to finishing
 *  up an operator.
 *
*/

void BMO_Finish_Op(BMesh *bm, BMOperator *op)
{
	BMOpSlot *slot;
	int i;

	for (i=0; i<opdefines[op->type]->totslot; i++) {
		slot = &op->slots[i];
		if (slot->slottype == BMOP_OPSLOT_MAPPING) {
			if (slot->data.ghash) 
				BLI_ghash_free(slot->data.ghash, NULL, NULL);
		}
	}

	BLI_memarena_free(op->arena);
}

/*
 * BMESH OPSTACK GET SLOT
 *
 * Returns a pointer to the slot of  
 * type 'slotcode'
 *
*/

BMOpSlot *BMO_GetSlot(BMOperator *op, int slotcode)
{
	return &(op->slots[slotcode]);
}

/*
 * BMESH OPSTACK COPY SLOT
 *
 * Copies data from one slot to another 
 *
*/

void BMO_CopySlot(BMOperator *source_op, BMOperator *dest_op, int src, int dst)
{
	BMOpSlot *source_slot = &source_op->slots[src];
	BMOpSlot *dest_slot = &dest_op->slots[dst];

	if(source_slot == dest_slot)
		return;

	if(source_slot->slottype != dest_slot->slottype)
		return;
	
	if(dest_slot->slottype > BMOP_OPSLOT_VEC){
		/*do buffer copy*/
		dest_slot->data.buf = NULL;
		dest_slot->len = source_slot->len;
		if(dest_slot->len){
			dest_slot->data.buf = BLI_memarena_alloc(dest_op->arena, BMOP_OPSLOT_TYPEINFO[dest_slot->slottype] * dest_slot->len);
			memcpy(dest_slot->data.buf, source_slot->data.buf, BMOP_OPSLOT_TYPEINFO[dest_slot->slottype] * dest_slot->len);
		}
	} else {
		dest_slot->data = source_slot->data;
	}
}

/*
 * BMESH OPSTACK SET XXX
 *
 * Sets the value of a slot depending on it's type
 *
*/


void BMO_Set_Float(BMOperator *op, int slotcode, float f)
{
	if( !(op->slots[slotcode].slottype == BMOP_OPSLOT_FLT) )
		return;
	op->slots[slotcode].data.f = f;
}

void BMO_Set_Int(BMOperator *op, int slotcode, int i)
{
	if( !(op->slots[slotcode].slottype == BMOP_OPSLOT_INT) )
		return;
	op->slots[slotcode].data.i = i;

}


void BMO_Set_Pnt(BMOperator *op, int slotcode, void *p)
{
	if( !(op->slots[slotcode].slottype == BMOP_OPSLOT_PNT) )
		return;
	op->slots[slotcode].data.p = p;
}

void BMO_Set_Vec(BMOperator *op, int slotcode, float *vec)
{
	if( !(op->slots[slotcode].slottype == BMOP_OPSLOT_VEC) )
		return;

	VECCOPY(op->slots[slotcode].data.vec, vec);
}

/*
 * BMO_SETFLAG
 *
 * Sets a flag for a certain element
 *
*/
void BMO_SetFlag(BMesh *bm, void *element, int flag)
{
	BMHeader *head = element;
	head->flags[bm->stackdepth-1].mask |= flag;
}

/*
 * BMO_CLEARFLAG
 *
 * Clears a specific flag from a given element
 *
*/

void BMO_ClearFlag(BMesh *bm, void *element, int flag)
{
	BMHeader *head = element;
	head->flags[bm->stackdepth-1].mask &= ~flag;
}

/*
 * BMO_TESTFLAG
 *
 * Tests whether or not a flag is set for a specific element
 *
 *
*/

int BMO_TestFlag(BMesh *bm, void *element, int flag)
{
	BMHeader *head = element;
	if(head->flags[bm->stackdepth-1].mask & flag)
		return 1;
	return 0;
}

/*
 * BMO_COUNTFLAG
 *
 * Counts the number of elements of a certain type that
 * have a specific flag set.
 *
*/

int BMO_CountFlag(BMesh *bm, int flag, int type)
{
	BMIter elements;
	BMHeader *e;
	int count = 0;

	if(type & BM_VERT){
		for(e = BMIter_New(&elements, bm, BM_VERTS, bm); e; e = BMIter_Step(&elements)){
			if(BMO_TestFlag(bm, e, flag))
				count++;
		}
	}
	if(type & BM_EDGE){
		for(e = BMIter_New(&elements, bm, BM_EDGES, bm); e; e = BMIter_Step(&elements)){
			if(BMO_TestFlag(bm, e, flag))
				count++;
		}
	}
	if(type & BM_FACE){
		for(e = BMIter_New(&elements, bm, BM_FACES, bm); e; e = BMIter_Step(&elements)){
			if(BMO_TestFlag(bm, e, flag))
				count++;
		}
	}

	return count;	
}

int BMO_CountSlotBuf(struct BMesh *bm, struct BMOperator *op, int slotcode)
{
	BMOpSlot *slot = &op->slots[slotcode];
	
	/*check if its actually a buffer*/
	if( !(slot->slottype > BMOP_OPSLOT_VEC) )
		return 0;

	return slot->len;
}

#if 0
void *BMO_Grow_Array(BMesh *bm, BMOperator *op, int slotcode, int totadd) {
	BMOpSlot *slot = &op->slots[slotcode];
	void *tmp;
	
	/*check if its actually a buffer*/
	if( !(slot->slottype > BMOP_OPSLOT_VEC) )
		return NULL;

	if (slot->flag & BMOS_DYNAMIC_ARRAY) {
		if (slot->len >= slot->size) {
			slot->size = (slot->size+1+totadd)*2;

			tmp = slot->data.buf;
			slot->data.buf = MEM_callocN(BMOP_OPSLOT_TYPEINFO[opdefines[op->type]->slottypes[slotcode]] * slot->size, "opslot dynamic array");
			memcpy(slot->data.buf, tmp, BMOP_OPSLOT_TYPEINFO[opdefines[op->type]->slottypes[slotcode]] * slot->size);
			MEM_freeN(tmp);
		}

		slot->len += totadd;
	} else {
		slot->flag |= BMOS_DYNAMIC_ARRAY;
		slot->len += totadd;
		slot->size = slot->len+2;
		tmp = slot->data.buf;
		slot->data.buf = MEM_callocN(BMOP_OPSLOT_TYPEINFO[opdefines[op->type]->slottypes[slotcode]] * slot->len, "opslot dynamic array");
		memcpy(slot->data.buf, tmp, BMOP_OPSLOT_TYPEINFO[opdefines[op->type]->slottypes[slotcode]] * slot->len);
	}

	return slot->data.buf;
}
#endif

void BMO_Insert_Mapping(BMesh *bm, BMOperator *op, int slotcode, 
			void *element, void *data, int len) {
	element_mapping *mapping;
	BMOpSlot *slot = &op->slots[slotcode];

	/*sanity check*/
	if (slot->slottype != BMOP_OPSLOT_MAPPING) return;
	
	mapping = BLI_memarena_alloc(op->arena, sizeof(*mapping) + len);

	mapping->element = element;
	mapping->len = len;
	memcpy(mapping+1, data, len);

	if (!slot->data.ghash) {
		slot->data.ghash = BLI_ghash_new(BLI_ghashutil_ptrhash, 
			                             BLI_ghashutil_ptrcmp);
	}
	
	BLI_ghash_insert(slot->data.ghash, element, mapping);
}

void BMO_Mapping_To_Flag(struct BMesh *bm, struct BMOperator *op, 
			 int slotcode, int flag)
{
	GHashIterator it;
	BMOpSlot *slot = &op->slots[slotcode];
	BMHeader *ele;

	/*sanity check*/
	if (slot->slottype != BMOP_OPSLOT_MAPPING) return;
	if (!slot->data.ghash) return;

	BLI_ghashIterator_init(&it, slot->data.ghash);
	for (;ele=BLI_ghashIterator_getKey(&it);BLI_ghashIterator_step(&it)) {
		BMO_SetFlag(bm, ele, flag);
	}
}

void BMO_Insert_MapFloat(BMesh *bm, BMOperator *op, int slotcode, 
			void *element, float val)
{
	BMO_Insert_Mapping(bm, op, slotcode, element, &val, sizeof(float));
}

void *BMO_Get_MapData(BMesh *bm, BMOperator *op, int slotcode,
		      void *element)
{
	element_mapping *mapping;
	BMOpSlot *slot = &op->slots[slotcode];

	/*sanity check*/
	if (slot->slottype != BMOP_OPSLOT_MAPPING) return NULL;
	if (!slot->data.ghash) return NULL;

	mapping = BLI_ghash_lookup(slot->data.ghash, element);
	
	return mapping + 1;
}

float BMO_Get_MapFloat(BMesh *bm, BMOperator *op, int slotcode,
		       void *element)
{
	float *val = BMO_Get_MapData(bm, op, slotcode, element);
	if (val) return *val;

	return 0.0f;
}

static void *alloc_slot_buffer(BMOperator *op, int slotcode, int len){

	/*check if its actually a buffer*/
	if( !(op->slots[slotcode].slottype > BMOP_OPSLOT_VEC) )
		return NULL;
	
	op->slots[slotcode].len = len;
	if(len)
		op->slots[slotcode].data.buf = BLI_memarena_alloc(op->arena, BMOP_OPSLOT_TYPEINFO[opdefines[op->type]->slottypes[slotcode]] * len);
	return op->slots[slotcode].data.buf;
}

/*
 *
 * BMO_HEADERFLAG_TO_SLOT
 *
 * Copies elements of a certain type, which have a certain header flag set 
 * into an output slot for an operator.
 *
*/

void BMO_HeaderFlag_To_Slot(BMesh *bm, BMOperator *op, int slotcode, int flag, int type)
{
	BMIter elements;
	BMHeader *e;
	BMOpSlot *output = BMO_GetSlot(op, slotcode);
	int totelement=0, i=0;
	
	totelement = BM_CountFlag(bm, type, BM_SELECT);

	if(totelement){
		alloc_slot_buffer(op, slotcode, totelement);

		if (type & BM_VERT) {
			for (e = BMIter_New(&elements, bm, BM_VERTS, bm); e; e = BMIter_Step(&elements)) {
				if(e->flag & flag) {
					((BMHeader**)output->data.p)[i] = e;
					i++;
				}
			}
		}

		if (type & BM_EDGE) {
			for (e = BMIter_New(&elements, bm, BM_EDGES, bm); e; e = BMIter_Step(&elements)) {
				if(e->flag & flag){
					((BMHeader**)output->data.p)[i] = e;
					i++;
				}
			}
		}

		if (type & BM_FACE) {
			for (e = BMIter_New(&elements, bm, BM_FACES, bm); e; e = BMIter_Step(&elements)) {
				if(e->flag & flag){
					((BMHeader**)output->data.p)[i] = e;
					i++;
				}
			}
		}
	}
}

/*
 *
 * BMO_FLAG_TO_SLOT
 *
 * Copies elements of a certain type, which have a certain flag set 
 * into an output slot for an operator.
 *
*/
void BMO_Flag_To_Slot(BMesh *bm, BMOperator *op, int slotcode, int flag, int type)
{
	BMIter elements;
	BMHeader *e;
	BMOpSlot *output = BMO_GetSlot(op, slotcode);
	int totelement = BMO_CountFlag(bm, flag, type), i=0;

	if(totelement){
		alloc_slot_buffer(op, slotcode, totelement);

		if (type & BM_VERT) {
			for (e = BMIter_New(&elements, bm, BM_VERTS, bm); e; e = BMIter_Step(&elements)) {
				if(BMO_TestFlag(bm, e, flag)){
					((BMHeader**)output->data.p)[i] = e;
					i++;
				}
			}
		}

		if (type & BM_EDGE) {
			for (e = BMIter_New(&elements, bm, BM_EDGES, bm); e; e = BMIter_Step(&elements)) {
				if(BMO_TestFlag(bm, e, flag)){
					((BMHeader**)output->data.p)[i] = e;
					i++;
				}
			}
		}

		if (type & BM_FACE) {
			for (e = BMIter_New(&elements, bm, BM_FACES, bm); e; e = BMIter_Step(&elements)) {
				if(BMO_TestFlag(bm, e, flag)){
					((BMHeader**)output->data.p)[i] = e;
					i++;
				}
			}
		}
	}
}

/*
 *
 * BMO_FLAG_BUFFER
 *
 * Flags elements in a slots buffer
 *
*/

void BMO_Flag_Buffer(BMesh *bm, BMOperator *op, int slotcode, int flag)
{
	BMOpSlot *slot = BMO_GetSlot(op, slotcode);
	BMHeader **data =  slot->data.p;
	int i;
	
	for(i = 0; i < slot->len; i++)
		BMO_SetFlag(bm, data[i], flag);
}


/*
 *
 *	ALLOC/FREE FLAG LAYER
 *
 *  Used by operator stack to free/allocate 
 *  private flag data. This is allocated
 *  using a mempool so the allocation/frees
 *  should be quite fast.
 *
 *  TODO:
 *	Investigate not freeing flag layers until
 *  all operators have been executed. This would
 *  save a lot of realloc potentially.
 *
*/

static void alloc_flag_layer(BMesh *bm)
{
	BMVert *v;
	BMEdge *e;
	BMFace *f;

	BMIter verts;
	BMIter edges;
	BMIter faces;
	BLI_mempool *oldpool = bm->flagpool; 		/*old flag pool*/
	void *oldflags;
	
	/*allocate new flag pool*/
	bm->flagpool = BLI_mempool_create(sizeof(BMFlagLayer)*(bm->totflags+1), 512, 512 );
	
	/*now go through and memcpy all the flags. Loops don't get a flag layer at this time...*/
	for(v = BMIter_New(&verts, bm, BM_VERTS, bm); v; v = BMIter_Step(&verts)){
		oldflags = v->head.flags;
		v->head.flags = BLI_mempool_calloc(bm->flagpool);
		memcpy(v->head.flags, oldflags, sizeof(BMFlagLayer)*bm->totflags); /*dont know if this memcpy usage is correct*/
	}
	for(e = BMIter_New(&edges, bm, BM_EDGES, bm); e; e = BMIter_Step(&edges)){
		oldflags = e->head.flags;
		e->head.flags = BLI_mempool_calloc(bm->flagpool);
		memcpy(e->head.flags, oldflags, sizeof(BMFlagLayer)*bm->totflags);
	}
	for(f = BMIter_New(&faces, bm, BM_FACES, bm); f; f = BMIter_Step(&faces)){
		oldflags = f->head.flags;
		f->head.flags = BLI_mempool_calloc(bm->flagpool);
		memcpy(f->head.flags, oldflags, sizeof(BMFlagLayer)*bm->totflags);
	}
	bm->totflags++;
	BLI_mempool_destroy(oldpool);
}

static void free_flag_layer(BMesh *bm)
{
	BMVert *v;
	BMEdge *e;
	BMFace *f;

	BMIter verts;
	BMIter edges;
	BMIter faces;
	BLI_mempool *oldpool = bm->flagpool;
	void *oldflags;
	
	/*de-increment the totflags first...*/
	bm->totflags--;
	/*allocate new flag pool*/
	bm->flagpool = BLI_mempool_create(sizeof(BMFlagLayer)*bm->totflags, 512, 512);
	
	/*now go through and memcpy all the flags*/
	for(v = BMIter_New(&verts, bm, BM_VERTS, bm); v; v = BMIter_Step(&verts)){
		oldflags = v->head.flags;
		v->head.flags = BLI_mempool_alloc(bm->flagpool);
		memcpy(v->head.flags, oldflags, sizeof(BMFlagLayer)*bm->totflags);  /*correct?*/
	}
	for(e = BMIter_New(&edges, bm, BM_EDGES, bm); e; e = BMIter_Step(&edges)){
		oldflags = e->head.flags;
		e->head.flags = BLI_mempool_alloc(bm->flagpool);
		memcpy(e->head.flags, oldflags, sizeof(BMFlagLayer)*bm->totflags);
	}
	for(f = BMIter_New(&faces, bm, BM_FACES, bm); f; f = BMIter_Step(&faces)){
		oldflags = f->head.flags;
		f->head.flags = BLI_mempool_alloc(bm->flagpool);
		memcpy(f->head.flags, oldflags, sizeof(BMFlagLayer)*bm->totflags);
	}

	BLI_mempool_destroy(oldpool);
}

static void clear_flag_layer(BMesh *bm)
{
	BMVert *v;
	BMEdge *e;
	BMFace *f;
	
	BMIter verts;
	BMIter edges;
	BMIter faces;
	
	/*now go through and memcpy all the flags*/
	for(v = BMIter_New(&verts, bm, BM_VERTS, bm); v; v = BMIter_Step(&verts)){
		memset(v->head.flags, 0, sizeof(BMFlagLayer)*bm->totflags);
	}
	for(e = BMIter_New(&edges, bm, BM_EDGES, bm); e; e = BMIter_Step(&edges)){
		memset(e->head.flags, 0, sizeof(BMFlagLayer)*bm->totflags);
	}
	for(f = BMIter_New(&faces, bm, BM_FACES, bm); f; f = BMIter_Step(&faces)){
		memset(f->head.flags, 0, sizeof(BMFlagLayer)*bm->totflags);
	}
}
