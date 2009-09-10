#include "MEM_guardedalloc.h"

#include "BKE_utildefines.h"

#include "BLI_memarena.h"
#include "BLI_mempool.h"
#include "BLI_heap.h"
#include "BLI_ghash.h"
#include "BLI_blenlib.h"
#include "BLI_arithb.h"

#include "bmesh.h"
#include "bmesh_operators_private.h"

#define ELE_NEW		1
#define ELE_OUT		2

typedef struct EPathNode {
	struct EPathNode *next, *prev;
	BMVert *v;
	BMEdge *e;
} EPathNode;

typedef struct EPath {
	ListBase nodes;
	float weight;
} EPath;

typedef struct PathBase {
	BLI_mempool *nodepool, *pathpool;
} PathBase;

typedef struct EdgeData {
	int tag;
	int ftag;
} EdgeData;

#define EDGE_MARK	1
#define EDGE_VIS	2

#define VERT_VIS	1

#define FACE_NEW	1

PathBase *edge_pathbase_new(void)
{
	PathBase *pb = MEM_callocN(sizeof(PathBase), "PathBase");

	pb->nodepool = BLI_mempool_create(sizeof(EPathNode), 1, 512);
	pb->pathpool = BLI_mempool_create(sizeof(EPath), 1, 512);

	return pb;
}

void edge_pathbase_free(PathBase *pathbase)
{
	BLI_mempool_destroy(pathbase->nodepool);
	BLI_mempool_destroy(pathbase->pathpool);
	MEM_freeN(pathbase);
}

EPath *edge_copy_add_path(PathBase *pb, EPath *path, BMVert *appendv, BMEdge *e)
{
	EPath *path2;
	EPathNode *node, *node2;

	path2 = BLI_mempool_calloc(pb->pathpool);
	
	for (node=path->nodes.first; node; node=node->next) {
		node2 = BLI_mempool_calloc(pb->nodepool);
		*node2 = *node;
		BLI_addtail(&path2->nodes, node2);
	}

	node2 = BLI_mempool_calloc(pb->nodepool);
	node2->v = appendv;
	node2->e = e;

	BLI_addtail(&path2->nodes, node2);

	return path2;
}

EPath *edge_path_new(PathBase *pb, BMVert *start)
{
	EPath *path;
	EPathNode *node;

	path = BLI_mempool_calloc(pb->pathpool);
	node = BLI_mempool_calloc(pb->nodepool);

	node->v = start;
	node->e = NULL;

	BLI_addtail(&path->nodes, node);
	path->weight = 0.0f;

	return path;
}

float edge_weight_path(EPath *path, EdgeData *edata)
{
	EPathNode *node;
	float w;

	for (node=path->nodes.first; node; node=node->next) {
		if (node->e) {
			w += edata[BMINDEX_GET(node->e)].ftag;
		}

		w += 1.0f;
	}

	return w;
}


void edge_free_path(PathBase *pathbase, EPath *path)
{
	EPathNode *node, *next;

	for (node=path->nodes.first; node; node=next) {
		next = node->next;
		BLI_mempool_free(pathbase->nodepool, node);
	}

	BLI_mempool_free(pathbase->pathpool, path);
}

EPath *edge_find_shortest_path(BMesh *bm, BMEdge *edge, EdgeData *edata, PathBase *pathbase)
{
	BMIter iter;
	BMEdge *e;
	GHash *gh = BLI_ghash_new(BLI_ghashutil_ptrhash, BLI_ghashutil_ptrcmp);
	BMVert *v1, *v2;
	BMVert **verts = NULL;
	V_DECLARE(verts);
	Heap *heap = BLI_heap_new();
	EPath *path = NULL, *path2;
	EPathNode *node;
	int i;

	path = edge_path_new(pathbase, edge->v1);
	BLI_heap_insert(heap, path->weight, path);
	path = NULL;

	while (BLI_heap_size(heap)) {
		if (path)
			edge_free_path(pathbase, path);
		path = BLI_heap_popmin(heap);
		v1 = ((EPathNode*)path->nodes.last)->v;
		
		if (v1 == edge->v2) {
			/*make sure this path loop doesn't already exist*/
			i = 0;
			V_RESET(verts);
			for (i=0, node = path->nodes.first; node; node=node->next, i++) {
				V_GROW(verts);
				verts[i] = node->v;
			}

			if (!BM_Face_Exists(bm, verts, i, NULL))
				break;
			else
				continue;
		}

		BM_ITER(e, &iter, bm, BM_EDGES_OF_VERT, v1) {
			if (e == edge || !BMO_TestFlag(bm, e, EDGE_MARK))
				continue;
			
			v2 = BM_OtherEdgeVert(e, v1);
			
			if (BLI_ghash_haskey(gh, v2))
				continue;

			BLI_ghash_insert(gh, v2, NULL);

			path2 = edge_copy_add_path(pathbase, path, v2, e);
			path2->weight = edge_weight_path(path2, edata);

			BLI_heap_insert(heap, path2->weight, path2);
		}

		if (BLI_heap_size(heap) == 0)
			path = NULL;
	}

	V_FREE(verts);
	BLI_heap_free(heap, NULL);
	BLI_ghash_free(gh, NULL, NULL);

	return path;
}

void bmesh_edgenet_fill_exec(BMesh *bm, BMOperator *op)
{
	BMIter iter, liter;
	BMOIter siter;
	BMEdge *e, *edge;
	BMLoop *l;
	BMFace *f;
	EPath *path;
	EPathNode *node;
	EdgeData *edata;
	BMEdge **edges = NULL;
	PathBase *pathbase = edge_pathbase_new();
	V_DECLARE(edges);
	int i, j;

	if (!bm->totvert || !bm->totedge)
		return;

	edata = MEM_callocN(sizeof(EdgeData)*bm->totedge, "EdgeData");
	BMO_Flag_Buffer(bm, op, "edges", EDGE_MARK, BM_EDGE);

	i = 0;
	BM_ITER(e, &iter, bm, BM_EDGES_OF_MESH, NULL) {
		BMINDEX_SET(e, i);
		
		if (!BMO_TestFlag(bm, e, EDGE_MARK)) {
			edata[i].tag = 2;
		}

		i += 1;
	}

	while (1) {
		edge = NULL;

		BMO_ITER(e, &siter, bm, op, "edges", BM_EDGE) {
			if (edata[BMINDEX_GET(e)].tag < 2) {
				edge = e;
				break;
			}
		}

		if (!edge)
			break;

		edata[BMINDEX_GET(edge)].tag += 1;

		path = edge_find_shortest_path(bm, edge, edata, pathbase);
		if (!path)
			continue;
		
		V_RESET(edges);
		i = 0;
		for (node=path->nodes.first; node; node=node->next) {
			if (!node->next)
				continue;

			e = BM_Edge_Exist(node->v, node->next->v);
			
			/*this should never happen*/
			if (!e)
				break;
			
			edata[BMINDEX_GET(e)].ftag++;
			V_GROW(edges);
			edges[i++] = e;
		}
		
		V_GROW(edges);
		edges[i++] = edge;

		f = BM_Make_Ngon(bm, edge->v1, edge->v2, edges, i, 1);
		if (f)
			BMO_SetFlag(bm, f, FACE_NEW);

		edge_free_path(pathbase, path);
	}

	BMO_Flag_To_Slot(bm, op, "faceout", FACE_NEW, BM_FACE);

	V_FREE(edges);
	edge_pathbase_free(pathbase);
	MEM_freeN(edata);
}

/* evaluate if entire quad is a proper convex quad */
static int convex(float *v1, float *v2, float *v3, float *v4)
{
	float nor[3], nor1[3], nor2[3], vec[4][2];
	
	/* define projection, do both trias apart, quad is undefined! */
	CalcNormFloat(v1, v2, v3, nor1);
	CalcNormFloat(v1, v3, v4, nor2);
	nor[0]= ABS(nor1[0]) + ABS(nor2[0]);
	nor[1]= ABS(nor1[1]) + ABS(nor2[1]);
	nor[2]= ABS(nor1[2]) + ABS(nor2[2]);

	if(nor[2] >= nor[0] && nor[2] >= nor[1]) {
		vec[0][0]= v1[0]; vec[0][1]= v1[1];
		vec[1][0]= v2[0]; vec[1][1]= v2[1];
		vec[2][0]= v3[0]; vec[2][1]= v3[1];
		vec[3][0]= v4[0]; vec[3][1]= v4[1];
	}
	else if(nor[1] >= nor[0] && nor[1]>= nor[2]) {
		vec[0][0]= v1[0]; vec[0][1]= v1[2];
		vec[1][0]= v2[0]; vec[1][1]= v2[2];
		vec[2][0]= v3[0]; vec[2][1]= v3[2];
		vec[3][0]= v4[0]; vec[3][1]= v4[2];
	}
	else {
		vec[0][0]= v1[1]; vec[0][1]= v1[2];
		vec[1][0]= v2[1]; vec[1][1]= v2[2];
		vec[2][0]= v3[1]; vec[2][1]= v3[2];
		vec[3][0]= v4[1]; vec[3][1]= v4[2];
	}
	
	/* linetests, the 2 diagonals have to instersect to be convex */
	if( IsectLL2Df(vec[0], vec[2], vec[1], vec[3]) > 0 ) return 1;
	return 0;
}

/*this is essentially new fkey*/
void bmesh_contextual_create_exec(BMesh *bm, BMOperator *op)
{
	BMOperator op2;
	BMOIter oiter;
	BMIter iter, liter;
	BMHeader *h;
	BMVert *v, *verts[4];
	BMEdge *e;
	BMLoop *l;
	BMFace *f;
	int totv=0, tote=0, totf=0, amount;

	/*count number of each element type we were passed*/
	BMO_ITER(h, &oiter, bm, op, "geom", BM_VERT|BM_EDGE|BM_FACE) {
		switch (h->type) {
			case BM_VERT: totv++; break;
			case BM_EDGE: tote++; break;
			case BM_FACE: totf++; break;
		}

		BMO_SetFlag(bm, h, ELE_NEW);
	}
	
	/*first call dissolve faces*/
	BMO_InitOpf(bm, &op2, "dissolvefaces faces=%ff", ELE_NEW);
	BMO_Exec_Op(bm, &op2);
	
	/*if we dissolved anything, then return.*/
	if (BMO_CountSlotBuf(bm, &op2, "regionout")) {
		BMO_CopySlot(&op2, op, "regionout", "faceout");
		BMO_Finish_Op(bm, &op2);
		return;
	}

	BMO_Finish_Op(bm, &op2);

	/*call edgenet create*/
	BMO_InitOpf(bm, &op2, "edgenet_fill edges=%fe", ELE_NEW);
	BMO_Exec_Op(bm, &op2);

	/*return if edge net create did something*/
	if (BMO_CountSlotBuf(bm, &op2, "faceout")) {
		BMO_CopySlot(&op2, op, "faceout", "faceout");
		BMO_Finish_Op(bm, &op2);
		return;
	}

	BMO_Finish_Op(bm, &op2);
	
	/*now, count how many verts we have*/
	amount = 0;
	BM_ITER(v, &iter, bm, BM_VERTS_OF_MESH, NULL) {
		if (BMO_TestFlag(bm, v, ELE_NEW)) {
			verts[amount] = v;
			amount++;

			if (amount > 4) break;
		}
	}

	if (amount == 2) {
		/*create edge*/
		e = BM_Make_Edge(bm, verts[0], verts[1], NULL, 1);
		BMO_SetFlag(bm, e, ELE_OUT);		
	} else if (amount == 3) {
		/*create triangle*/
		BM_Make_QuadTri(bm, verts[0], verts[1], verts[2], NULL, NULL, 1);
	} else if (amount == 4) {
		f = NULL;

		/* the order of vertices can be anything, 6 cases to check */
		if( convex(verts[0]->co, verts[1]->co, verts[2]->co, verts[3]->co) ) {
			f= BM_Make_QuadTri(bm, verts[0], verts[1], verts[2], verts[3], NULL, 1);
		}
		else if( convex(verts[0]->co, verts[2]->co, verts[3]->co, verts[1]->co) ) {
			f= BM_Make_QuadTri(bm, verts[0], verts[2], verts[3], verts[1], NULL, 1);
		}
		else if( convex(verts[0]->co, verts[2]->co, verts[1]->co, verts[3]->co) ) {
			f= BM_Make_QuadTri(bm, verts[0], verts[2], verts[1], verts[3], NULL, 1);
		}
		else if( convex(verts[0]->co, verts[1]->co, verts[3]->co, verts[2]->co) ) {
			f= BM_Make_QuadTri(bm, verts[0], verts[1], verts[3], verts[2], NULL, 1);
		}
		else if( convex(verts[0]->co, verts[3]->co, verts[2]->co, verts[1]->co) ) {
			f= BM_Make_QuadTri(bm, verts[0], verts[3], verts[2], verts[1], NULL, 1);
		}
		else if( convex(verts[0]->co, verts[3]->co, verts[1]->co, verts[2]->co) ) {
			f= BM_Make_QuadTri(bm, verts[0], verts[3], verts[1], verts[2], NULL, 1);
		}
		else {
			printf("cannot find nice quad from concave set of vertices\n");
		}

		if (f) BMO_SetFlag(bm, f, ELE_OUT);
	}
}
