/**********
*  Red-Black Tree Template in C
*  Author: Acciente717
*  Version: 1.0
*  Date: 2017-11-30
***********/

/**********
*  Template Block
*  ONLY modify this block when using the code.
***********/

// define the data structure stored in rb-tree
typedef struct __rb_data
{
	int val;
};

// define the comparator of the data stucture stored in rb-tree
inline static int __rb_data_less(struct __rb_data *x, struct __rb_data *y)
{
	return x->val < y->val;
}

/**********
*  End Template Block
***********/


/**********
*  Tree Structure Defination Block
***********/

enum __rb_color { __rb_BLACK, __rb_RED };

typedef struct __rb_node
{
	struct __rb_node *lc, *rc, *pa;
	__rb_color color;
	__rb_data dat;
};

/**********
*  End Tree Structure Defination Block
***********/


/**********
*  Global Variable Block
***********/

__rb_node __rb_tmp_node;

struct __rb_node *__rb_root = NULL;
struct __rb_node *__rb_NIL = NULL;

/**********
*  End Global Variable Block
***********/



/**********
*  Private Function Block
***********/

static void __rb_left_rotate(struct __rb_node *x)
{
	struct __rb_node *y = x->rc;
	x->rc = y->lc;
	if (y->lc != __rb_NIL)
		y->lc->pa = x;
	y->pa = x->pa;
	if (x->pa == __rb_NIL)
		__rb_root = y;
	else if (x == x->pa->lc)
		x->pa->lc = y;
	else
		x->pa->rc = y;
	y->lc = x;
	x->pa = y;
}

static void __rb_right_rotate(struct __rb_node *x)
{
	struct __rb_node *y = x->lc;
	x->lc = y->rc;
	if (y->rc != __rb_NIL)
		y->rc->pa = x;
	y->pa = x->pa;
	if (x->pa == __rb_NIL)
		__rb_root = y;
	else if (x == x->pa->lc)
		x->pa->lc = y;
	else
		x->pa->rc = y;
	y->rc = x;
	x->pa = y;
}

static void __rb_insert_fixup(struct __rb_node *z)
{
	struct __rb_node *x, *y;
	while (z->pa->color == __rb_RED)
	{
		if (z->pa == z->pa->pa->lc)
		{
			y = z->pa->pa->rc;
			if (y->color == __rb_RED)
			{
				z->pa->color = __rb_BLACK;
				y->color = __rb_BLACK;
				z->pa->pa->color = __rb_RED;
				z = z->pa->pa;
			}
			else
			{
				if (z == z->pa->rc)
				{
					z = z->pa;
					__rb_left_rotate(z);
				}
				z->pa->color = __rb_BLACK;
				z->pa->pa->color = __rb_RED;
				__rb_right_rotate(z->pa->pa);
			}
		}
		else
		{
			y = z->pa->pa->lc;
			if (y->color == __rb_RED)
			{
				z->pa->color = __rb_BLACK;
				y->color = __rb_BLACK;
				z->pa->pa->color = __rb_RED;
				z = z->pa->pa;
			}
			else
			{
				if (z == z->pa->lc)
				{
					z = z->pa;
					__rb_right_rotate(z);
				}
				z->pa->color = __rb_BLACK;
				z->pa->pa->color = __rb_RED;
				__rb_left_rotate(z->pa->pa);
			}
		}
	}
	__rb_root->color = __rb_BLACK;
}

static void __rb_insert(struct __rb_node *z)
{
	struct __rb_node *x = __rb_root, *y = __rb_NIL;
	while (x != __rb_NIL)
	{
		y = x;
		if (__rb_data_less(&z->dat, &x->dat))
			x = x->lc;
		else
			x = x->rc;
	}
	z->pa = y;
	if (y == __rb_NIL)
		__rb_root = z;
	else if (__rb_data_less(&z->dat, &y->dat))
		y->lc = z;
	else
		y->rc = z;
	z->lc = z->rc = __rb_NIL;
	z->color = __rb_RED;
	__rb_insert_fixup(z);
}

inline static void __rb_transplant(struct __rb_node *u, struct __rb_node *v)
{
	if (u->pa == __rb_NIL)
		__rb_root = v;
	else if (u == u->pa->lc)
		u->pa->lc = v;
	else
		u->pa->rc = v;
	v->pa = u->pa;
}

static void __rb_delete_fixup(struct __rb_node *x)
{
	struct __rb_node *w;
	while (x != __rb_root && x->color == __rb_BLACK)
	{
		if (x == x->pa->lc)
		{
			w = x->pa->rc;
			if (w->color == __rb_RED)
			{
				w->color = __rb_BLACK;
				x->pa->color = __rb_RED;
				__rb_left_rotate(x->pa);
				w = x->pa->rc;
			}
			if (w->lc->color == __rb_BLACK && w->rc->color == __rb_BLACK)
			{
				w->color = __rb_RED;
				x = x->pa;
			}
			else
			{
				if (w->rc->color == __rb_BLACK)
				{
					w->lc->color = __rb_BLACK;
					w->color = __rb_RED;
					__rb_right_rotate(w);
					w = x->pa->rc;
				}
				w->color = x->pa->color;
				x->pa->color = __rb_BLACK;
				w->rc->color = __rb_BLACK;
				__rb_left_rotate(x->pa);
				x = __rb_root;
			}
		}
		else
		{
			w = x->pa->lc;
			if (w->color == __rb_RED)
			{
				w->color = __rb_BLACK;
				x->pa->color = __rb_RED;
				__rb_right_rotate(x->pa);
				w = x->pa->lc;
			}
			if (w->rc->color == __rb_BLACK && w->lc->color == __rb_BLACK)
			{
				w->color = __rb_RED;
				x = x->pa;
			}
			else
			{
				if (w->lc->color == __rb_BLACK)
				{
					w->rc->color = __rb_BLACK;
					w->color = __rb_RED;
					__rb_left_rotate(w);
					w = x->pa->lc;
				}
				w->color = x->pa->color;
				x->pa->color = __rb_BLACK;
				w->lc->color = __rb_BLACK;
				__rb_right_rotate(x->pa);
				x = __rb_root;
			}
		}
	}
	x->color = __rb_BLACK;
}

inline static struct __rb_node * __rb_tree_minimum(struct __rb_node *x)
{
	while (x->lc != __rb_NIL)
		x = x->lc;
	return x;
}

static void __rb_delete(struct __rb_node *z)
{
	struct __rb_node *x, *y = z;
	__rb_color y_original_color = y->color;
	if (z->lc == __rb_NIL)
	{
		x = z->rc;
		__rb_transplant(z, z->rc);
	}
	else if (z->rc == __rb_NIL)
	{
		x = z->lc;
		__rb_transplant(z, z->lc);
	}
	else
	{
		y = __rb_tree_minimum(z->rc);
		y_original_color = y->color;
		x = y->rc;
		if (y->pa == z)
			x->pa = y;
		else
		{
			__rb_transplant(y, y->rc);
			y->rc = z->rc;
			y->rc->pa = y;
		}
		__rb_transplant(z, y);
		y->lc = z->lc;
		y->lc->pa = y;
		y->color = z->color;
	}
	free(z);
	if (y_original_color == __rb_BLACK)
		__rb_delete_fixup(x);
}

void __rb_recur_destruct(struct __rb_node *ptr)
{
	if (ptr == __rb_NIL) return;
	if (ptr->lc != __rb_NIL) __rb_recur_destruct(ptr->lc);
	if (ptr->rc != __rb_NIL) __rb_recur_destruct(ptr->rc);
	free(ptr);
}

/**********
*  End Private Function Block
***********/



/**********
*  Public Interface Block
***********/

void rb_init_tree()
{
	__rb_NIL = (struct __rb_node*)malloc(sizeof(struct __rb_node));
	__rb_root = __rb_NIL;
	__rb_NIL->color = __rb_BLACK;
}

void rb_destruct_tree()
{
	__rb_recur_destruct(__rb_root);
	__rb_root = NULL;
	free(__rb_NIL);
	__rb_NIL = NULL;
}

void rb_insert_data(struct __rb_data ins_data)
{
	struct __rb_node *z;
	z = (struct __rb_node *)malloc(sizeof(struct __rb_node));
	z->dat = ins_data;
	__rb_insert(z);
}

struct __rb_node * rb_find(struct __rb_data find_data)
{
	struct __rb_node * pos = __rb_root;
	while (pos)
	{
		if (__rb_data_less(&find_data, &pos->dat))
			pos = pos->lc;
		else if (__rb_data_less(&pos->dat, &find_data))
			pos = pos->rc;
		else
			break;
	}
	return pos;
}

// return 1 on successful deletion, otherwise 0
int rb_delete_data(struct __rb_data del_data)
{
	struct __rb_node *ptr;
	ptr = rb_find(del_data);
	if (ptr != __rb_NIL)
	{
		__rb_delete(ptr);
		return 1;
	}
	return 0;
}

void rb_middle_traverse(struct __rb_node *ptr)
{
	if (ptr == __rb_NIL) return;
	if (ptr->lc != __rb_NIL)
		rb_middle_traverse(ptr->lc);
	printf("%d ", ptr->dat.val);
	if (ptr->rc != __rb_NIL)
		rb_middle_traverse(ptr->rc);
}

/**********
*  End Public Interface Block
***********/
