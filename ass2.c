/* Solution to comp10002 Assignment 2, 2019 semester 2.

   Authorship Declaration:

   (1) I certify that the program contained in this submission is completely
   my own individual work, except where explicitly noted by comments that
   provide details otherwise.  I understand that work that has been developed
   by another student, or by me in collaboration with other students,
   or by non-students as a result of request, solicitation, or payment,
   may not be submitted for assessment in this subject.  I understand that
   submitting for assessment work developed by or in collaboration with
   other students or non-students constitutes Academic Misconduct, and
   may be penalized by mark deductions, or by other penalties determined
   via the University of Melbourne Academic Honesty Policy, as described
   at https://academicintegrity.unimelb.edu.au.

   (2) I also certify that I have not provided a copy of this work in either
   softcopy or hardcopy or any other form to any other student, and nor will
   I do so until after the marks are released. I understand that providing
   my work to other students, regardless of my intention or any undertakings
   made to me by that other student, is also Academic Misconduct.

   (3) I further understand that providing a copy of the assignment
   specification to any form of code authoring or assignment tutoring
   service, or drawing the attention of others to such services and code
   that may have been made available via such a service, may be regarded
   as Student General Misconduct (interfering with the teaching activities
   of the University and/or inciting others to commit Academic Misconduct).
   I understand that an allegation of Student General Misconduct may arise
   regardless of whether or not I personally make use of such solutions
   or sought benefit from such actions.

   Signed by: Christopher Chamberlain 1082551
   Dated:     3/10/19

*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#define DEBUG 1
#define VEC_INITIAL_CAPACITY 2

#define ROUTE_VALID 0
#define INITIAL_CELL_WRONG 1
#define GOAL_CELL_WRONG 2
#define ILLEGAL_MOVE 3
#define BLOCK_IN_ROUTE 4

#define EMPTY_CELL -1
#define BLOCK_CELL -2
#define INITIAL_CELL -3
#define GOAL_CELL -4
#define ROUTE_CELL -5
#define SEARCH_CELL_MIN 0

typedef struct {
    int rows;
    int cols;
} dimensions_t;

typedef struct {
    int x;
    int y;
} point_t;

/* path-finding grid */

typedef int cell_t;

typedef struct {
    dimensions_t dim;
    int num_blocks;
    cell_t *inner;
} grid_t;

void grid_alloc(grid_t *grid, dimensions_t dim);
cell_t *grid_get(grid_t *grid, point_t point);
void grid_copy(grid_t *src, grid_t *dest);
void grid_free(grid_t *grid);

/* linked-list */

struct node;

typedef struct node node_t;
typedef point_t list_item_t;

struct node {
    list_item_t coords;
    node_t *next;
    node_t *prev;
};

typedef struct {
    node_t *head;
    node_t *foot;
} list_t;

list_t* ll_alloc();
bool ll_is_empty(list_t *list);
node_t *ll_insert_before(list_t* list, node_t *node, list_item_t data);
node_t *ll_insert_after(list_t* list, node_t *node, list_item_t data);
void ll_remove(list_t *list, node_t *node);
void ll_free(list_t *list);

/* vector, used as a queue for path-repair */

typedef struct {
    point_t point;
    int distance;
} vec_item_t;

typedef struct {
    vec_item_t *storage;
    size_t len;
    size_t capacity;
} vec_t;

void vec_alloc(vec_t *vec);
void vec_push(vec_t *vec, vec_item_t item);
vec_item_t *vec_get(vec_t *vec, size_t idx);
void print_vec(vec_t *vec);
void vec_free(vec_t *vec);


dimensions_t parse_dimensions();
void print_results(grid_t *grid, point_t start, point_t end, list_t *path,
                   int status, bool initial);
void print_grid(grid_t *grid);
int verify_route(grid_t *grid,
                 point_t start, point_t end, list_t *path);
void print_point(point_t point);
bool repair_route(grid_t *grid, list_t *path);
bool try_repair_step(grid_t *grid, point_t point, int distance, vec_t *repair, point_t *repair_end);
void try_backtrack(grid_t *grid, point_t *point, int *min_distance, point_t try_point);
void fill_grid(grid_t *grid, point_t start, point_t end, list_t *path);

bool parse_point(point_t *point);
bool same_point(point_t a, point_t b);


int
main(int argc, char *argv[]) {
    dimensions_t dim = parse_dimensions();

    grid_t grid;
    grid_alloc(&grid, dim);

    point_t start;
    parse_point(&start);
    scanf("\n");
    point_t end;
    parse_point(&end);
    scanf("\n");

    point_t point;
    while(parse_point(&point)) {
        *grid_get(&grid, point) = BLOCK_CELL;
        grid.num_blocks++;
        scanf("\n");
    }

    scanf("$\n");

    list_t* path = ll_alloc();

    while(parse_point(&point)) {
        ll_insert_after(path, path->foot, point);
        scanf("->");
        scanf("\n");
    }

    int status = verify_route(&grid, start, end, path);

    /* stage 1 results */
    printf("==STAGE 0=======================================\n");
    print_results(&grid, start, end, path, status, true);

    printf("==STAGE 1=======================================\n");
    grid_t filled_grid;
    grid_copy(&grid, &filled_grid);
    fill_grid(&filled_grid, start, end, path);
    print_grid(&filled_grid);
    grid_free(&filled_grid);

    if(status == BLOCK_IN_ROUTE) {
        printf("------------------------------------------------\n");
        /* we can discard `start` and `end` as they must be equal to
           path->head and path->foot correct at this point. */
        grid_t repair_grid;
        grid_copy(&grid, &repair_grid);
        bool repaired = repair_route(&repair_grid, path);
        grid_free(&repair_grid);

        if(repaired) {
            grid_copy(&grid, &filled_grid);
            fill_grid(&filled_grid, start, end, path);
            print_grid(&filled_grid);
            grid_free(&filled_grid);
            printf("------------------------------------------------\n");
            print_results(&grid, start, end, path, ROUTE_VALID, false);
        } else {
            printf("The route cannot be repaired!\n");
        }
    }

    printf("================================================\n");

    ll_free(path);
    grid_free(&grid);
	return 0;
}

int verify_route(grid_t *grid,
                 point_t start, point_t end, list_t *path) {
    assert(!ll_is_empty(path));
    if(!same_point(path->head->coords, start)) {
        return INITIAL_CELL_WRONG;
    } else if(!same_point(path->foot->coords, end)) {
        return GOAL_CELL_WRONG;
    }

    node_t *step = path->head;
    bool block_in_route = false;
    while(step != NULL && step->next != NULL) {
        /* only allowed to move one cell in either direction each step */
        int difference = abs(step->coords.x - step->next->coords.x) +
                         abs(step->coords.y - step->next->coords.y);
        if(*grid_get(grid, step->coords) == BLOCK_CELL) {
            block_in_route = true;
        }
        if(difference > 1) {
            return ILLEGAL_MOVE;
        }

        step = step->next;
    }

    if(block_in_route) {
        return BLOCK_IN_ROUTE;
    }

    return ROUTE_VALID;
}

bool repair_route(grid_t *grid, list_t *path) {
    assert(!ll_is_empty(path));
    node_t *step = path->head;
    while(step->next != NULL && *grid_get(grid, step->next->coords) != BLOCK_CELL) {
        step = step->next;
    }

    vec_item_t repair_start = {
        .point = step->coords,
        .distance = 0
    };
    printf("Starting repair at: ");
    print_point(repair_start.point);
    *grid_get(grid, repair_start.point) = SEARCH_CELL_MIN;

    /* skip the starting cell */
    step = step->next;
    while(step != NULL && *grid_get(grid, step->coords) == BLOCK_CELL) {
        step = step->next;
        ll_remove(path, step->prev);
    }
    printf(", ending repair at: ");
    node_t *insert_before = step;

    /* construct a cheeky 'view' into the linked list, that only contains the
       path *after* the gap */
    list_t tail_path = { .head = insert_before, .foot = path->foot };
    fill_grid(grid, path->head->coords, path->foot->coords, &tail_path);

    print_point(insert_before->coords);
    printf("\n");

    vec_t repair;
    vec_alloc(&repair);
    vec_push(&repair, repair_start);

    point_t repair_end;
    int dist;
    size_t i = 0;
    while(i < repair.len) {
        vec_item_t *current = vec_get(&repair, i);
        point_t p = current->point;
        point_t above = { .x = p.x, .y = p.y - 1};
        point_t below = { .x = p.x, .y = p.y + 1};
        point_t left = { .x = p.x -1, .y = p.y };
        point_t right = { .x = p.x + 1, .y = p.y };
        dist = current->distance + 1;
        if(above.y >= 0 && try_repair_step(grid, above, dist, &repair, &repair_end)) {
            break;
        }
        if(below.y < grid->dim.rows && try_repair_step(grid, below, dist, &repair, &repair_end)) {
            break;
        }
        if(left.x >= 0 && try_repair_step(grid, left, dist, &repair, &repair_end)) {
            break;
        }
        if(right.x < grid->dim.cols && try_repair_step(grid, right, dist, &repair, &repair_end)) {
            break;
        }
        i++;
    }

    if(i >= repair.len) {
        return false;
    }

    point_t p = repair_end;
    while(!same_point(p, repair_start.point)) {
        point_t above = { .x = p.x, .y = p.y - 1};
        point_t below = { .x = p.x, .y = p.y + 1};
        point_t left = { .x = p.x -1, .y = p.y };
        point_t right = { .x = p.x + 1, .y = p.y };
        if(right.x < grid->dim.cols) {
            try_backtrack(grid, &p, &dist, right);
        }
        if(left.x >= 0) {
            try_backtrack(grid, &p, &dist, left);
        }
        if(below.y < grid->dim.rows) {
            try_backtrack(grid, &p, &dist, below);
        }
        if(above.y >= 0) {
            try_backtrack(grid, &p, &dist, above);
        }

        insert_before = ll_insert_before(path, insert_before, p);
        i++;
    }

    return true;
}

bool try_repair_step(grid_t *grid, point_t point, int distance, vec_t *repair, point_t *repair_end) {
    int *cell = grid_get(grid, point);
    if(*cell == EMPTY_CELL) {
        vec_item_t next = { .distance = distance, .point = point };
        *grid_get(grid, point) = SEARCH_CELL_MIN + next.distance;
        vec_push(repair, next);
    } else if(*cell == ROUTE_CELL || *cell == GOAL_CELL) {
        *repair_end = point;
        return true;
    }
    return false;
}

void try_backtrack(grid_t *grid, point_t *point, int *min_distance, point_t try_point) {
    int cell = *grid_get(grid, try_point);
    if(cell >= SEARCH_CELL_MIN) {
        int distance = cell - SEARCH_CELL_MIN;
        if(distance <= *min_distance) {
            *point = try_point;
            *min_distance = distance;
        }
    }
}

void fill_grid(grid_t *grid, point_t start, point_t end, list_t *path) {
    /* to visualise the grid, we first copy the path into the 2D array*/
    node_t *step = path->head;
    while(step != NULL) {
        cell_t *cell = grid_get(grid, step->coords);
        if(*cell != BLOCK_CELL) {
            *cell = ROUTE_CELL;
        }
        step = step->next;
    }

    *grid_get(grid, start) = INITIAL_CELL;
    *grid_get(grid, end) = GOAL_CELL;
}

void print_grid(grid_t *grid) {
    int i, j;
    for(j = -1; j < grid->dim.rows; j++) {
        for(i = -1; i < grid->dim.cols; i++) {
            if(i == -1 && j == -1) {
                printf(" ");
            } else if(i == -1) {
                printf("%d", j % 10);
            } else if(j == -1) {
                printf("%d", i % 10);
            } else {
                point_t coords = { .x = i, .y = j };
                cell_t cell = *grid_get(grid, coords);
                if(cell == EMPTY_CELL) { printf(" "); }
                else if(cell == BLOCK_CELL) { printf("#"); }
                else if(cell == ROUTE_CELL) { printf("*"); }
                else if(cell == INITIAL_CELL) { printf("I"); }
                else if(cell == GOAL_CELL) { printf("G"); }
                else if(cell >= SEARCH_CELL_MIN) { printf("%d", cell - SEARCH_CELL_MIN); }
            }
        }
        printf("\n");
    }
}

void print_point(point_t point) {
    printf("[%d,%d]", point.y, point.x);
}

void print_results(grid_t *grid, point_t start,
                   point_t end, list_t *path, int status, bool initial) {
    if(initial) {
        printf("The grid has %d rows and %d columns.\n", grid->dim.rows, grid->dim.cols);
        printf("The grid has %d block(s).\n", grid->num_blocks);
        printf("The initial cell in the grid is ");
        print_point(start);
        printf(".\nThe goal cell in the grid is ");
        print_point(end);
        printf(".\nThe proposed route in the grid is:\n");
    }
    node_t *step = path->head;
    int num_on_line = 0;
    while(step != NULL) {
        if(num_on_line >= 5) {
            num_on_line = 0;
            printf("\n");
        }
        print_point(step->coords);
        step = step->next;
        num_on_line++;
        if(step != NULL) {
            printf("->");
        } else {
            printf(".");
        }
    }
    printf("\n");

    if(status == ROUTE_VALID) {
        printf("The route is valid!\n");
    } else if(status == INITIAL_CELL_WRONG) {
        printf("Initial cell in the route is wrong!\n");
    } else if(status == GOAL_CELL_WRONG) {
        printf("Goal cell in the route is wrong!\n");
    } else if(status == ILLEGAL_MOVE) {
        printf("There is an illegal move in this route!\n");
    } else if(status == BLOCK_IN_ROUTE) {
        printf("There is a block on this route!\n");
    }
}

dimensions_t parse_dimensions() {
    dimensions_t dim;
    int res = scanf("%dx%d\n", &dim.rows, &dim.cols);
    if(res != 2) {
        fprintf(stderr, "Failed to parse grid dimensions from stdin.\n");
        exit(EXIT_FAILURE);
    }
    return dim;
}

bool parse_point(point_t *point) {
    /* [r, c] where r is the row and c is the column */
    int res = scanf("[%d,%d]", &point->y, &point->x);
    if(res != 2) {
        return false;
    }
    return true;
}

bool same_point(point_t a, point_t b) {
    return a.x == b.x && a.y == b.y;
}

/* allocates space for a grid and sets all cells to EMPTY_CELL */
void grid_alloc(grid_t *grid, dimensions_t dim) {
    grid->dim = dim;
    grid->num_blocks = 0;
    size_t bytes = sizeof(cell_t) * dim.rows * dim.cols;
    grid->inner = (cell_t *) malloc(bytes);
    assert(grid->inner != NULL);
    /* 'zeroes' the memory with empty cells */
    memset(grid->inner, EMPTY_CELL, bytes);
}

cell_t *grid_get(grid_t *grid, point_t point) {
    assert(grid->inner != NULL);
    assert(point.x < grid->dim.cols && point.y < grid->dim.rows);
    return &grid->inner[point.y * grid->dim.rows + point.x];
}

/* NOTE: if 'dest' was already an allocated grid, it must be freed before
   first otherwise memory will be leaked */
void grid_copy(grid_t *src, grid_t *dest) {
    grid_alloc(dest, src->dim);
    dest->num_blocks = src->num_blocks;
    size_t bytes = sizeof(cell_t) * src->dim.rows * src->dim.cols;
    memcpy(dest->inner, src->inner, bytes);
}

void grid_free(grid_t *grid) {
    free(grid->inner);
    grid->inner = NULL;
}

list_t* ll_alloc() {
    list_t *list = (list_t *) malloc(sizeof(*list));
    assert(list != NULL);
    list->head = NULL;
    list->foot = NULL;
    return list;
}

bool ll_is_empty(list_t *list) {
    return list->head == NULL;
}

node_t *ll_insert_before(list_t* list, node_t *before, list_item_t coords) {
    node_t *node = (node_t *) malloc(sizeof(*node));
    assert(node != NULL);
    node->coords = coords;
    node->next = before;

    if(before == NULL) {
        /* the list must be empty */
        assert(ll_is_empty(list));
        list->head = node;
        list->foot = node;
        node->prev = NULL;
    } else {
        node->prev = before->prev;
        if(list->head == before) {
            list->head = node;
        } else{
            assert(node->prev != NULL);
            node->prev->next = node;
        }
        before->prev = node;
    }

    return node;
}

node_t *ll_insert_after(list_t* list, node_t *after, list_item_t coords) {
    node_t *node = (node_t *) malloc(sizeof(*node));
    assert(node != NULL);
    node->coords = coords;
    node->prev = after;

    if(after == NULL) {
        /* the list must be empty */
        assert(ll_is_empty(list));
        list->head = node;
        list->foot = node;
        node->next = NULL;
    } else {
        node->next = after->next;
        if(list->foot == after) {
            list->foot = node;
        } else{
            assert(node->next != NULL);
            node->next->prev = node;
        }
        after->next = node;
    }

    return node;
}

void ll_remove(list_t *list, node_t *node) {
    if(list->head == node && list->foot == node) {
        list->head = NULL;
        list->foot = NULL;
    } else if(list->head == node) {
        node->next->prev = NULL;
        list->head = node->next;
    } else if(list->foot == node) {
        node->prev->next = NULL;
        list->foot = node->prev;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    free(node);
}

void ll_free(list_t *list) {
    node_t* node = list->head;
    while(node != NULL) {
        node_t *data = node;
        node = node->next;
        free(data);
    }

    free(list);
    list = NULL;
}


void vec_alloc(vec_t *vec) {
    vec->storage = (vec_item_t *) malloc(sizeof(vec_item_t) * VEC_INITIAL_CAPACITY);
    assert(vec->storage != NULL);
    vec->len = 0;
    vec->capacity = 2;
}

void vec_push(vec_t *vec, vec_item_t item) {
    if(vec->len == vec->capacity) {
        /* reallocate */
        vec->capacity *= 2;
        vec->storage = (vec_item_t *) realloc(vec->storage, sizeof(vec_item_t)
                                            * vec->capacity);
    }
    vec->storage[vec->len++] = item;
}

vec_item_t *vec_get(vec_t *vec, size_t idx) {
    assert(idx < vec->len);
    return &vec->storage[idx];
}

void print_vec(vec_t *vec) {
    int i;
    for(i = 0; i < vec->len; i++) {
        print_point(vec->storage[i].point);
        printf(": #%d\n", vec->storage[i].distance);
    }
}

void vec_free(vec_t *vec) {
    free(vec->storage);
    vec->storage = NULL;
}