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

#define DEBUG 0
#define VEC_INITIAL_CAPACITY 2
#define DSEPARATOR "================================================\n"
#define SSEPARATOR "------------------------------------------------\n"

#define ROUTE_VALID 5
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

void grid_init(grid_t *grid, dimensions_t dim);
void grid_clear(grid_t *grid);
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

void ll_init(list_t *list);
bool ll_is_empty(list_t *list);
node_t *ll_insert_before(list_t* list, node_t *node, list_item_t data);
node_t *ll_insert_after(list_t* list, node_t *node, list_item_t data);
void ll_remove(list_t *list, node_t *node);
void ll_free(list_t *list);

/* vector, used as a queue for path-repair */

typedef struct {
    point_t coords;
    int distance;
} vec_item_t;

typedef struct {
    vec_item_t *storage;
    size_t len;
    size_t capacity;
} vec_t;

void vec_init(vec_t *vec);
void vec_push(vec_t *vec, vec_item_t item);
vec_item_t *vec_get(vec_t *vec, size_t idx);
void print_vec(vec_t *vec);
void vec_free(vec_t *vec);

typedef struct {
    point_t start;
    point_t end;
    list_t steps;
} path_t;

dimensions_t parse_dimensions();
void print_results(grid_t *grid, path_t *path,
                   int status, bool initial);
void print_list(list_t *list);
void print_grid(grid_t *grid);
void print_path_on_grid(grid_t *grid, grid_t *scratch_grid, path_t *path);
int verify_route(grid_t *grid, path_t *path);
void print_point(point_t point);
void stage_one(grid_t *grid, grid_t *scratch_grid, path_t *path, int *status);
void iter_repair_route(grid_t *grid, grid_t* scratch_grid, path_t* path, int *status, bool iter);
bool repair_route(grid_t *grid, path_t *path);
bool try_repair_step(grid_t *grid, point_t point, int distance, vec_t *repair, point_t *repair_end);
void try_backtrack(grid_t *grid, point_t *point, int *min_distance, point_t try_point);
void draw_path(grid_t *grid, node_t *start_path);

bool parse_point(point_t *point);
bool same_point(point_t a, point_t b);


int
main(int argc, char *argv[]) {
    dimensions_t dim = parse_dimensions();

    grid_t grid;
    grid_init(&grid, dim);

    point_t start;
    int res = parse_point(&start);
    assert(res);
    scanf("\n");
    point_t end;
    res = parse_point(&end);
    assert(res);
    scanf("\n");

    point_t point;
    while(parse_point(&point)) {
        *grid_get(&grid, point) = BLOCK_CELL;
        grid.num_blocks++;
        scanf(" ");
    }

    scanf("$ ");

    path_t path = {
        .start = start,
        .end = end
    };
    ll_init(&path.steps);

    while(parse_point(&point)) {
        ll_insert_after(&path.steps, path.steps.foot, point);
        scanf("-> "); /* allows for whitespace e.g.: newlines after an arrow */
    }

    int status = verify_route(&grid, &path);
    grid_t scratch_grid;
    grid_init(&scratch_grid, grid.dim);

    /* stage 1 results */
    printf("==STAGE 0=======================================\n");
    print_results(&grid, &path, status, true);

    printf("==STAGE 1=======================================\n");
    iter_repair_route(&grid, &scratch_grid, &path, &status, false);

    bool first = true;
    /* consume any whitespace */
    scanf(" ");
    while((status == ROUTE_VALID || status == BLOCK_IN_ROUTE) && getchar() == '$') {
        if(first) {
            printf("==STAGE 2=======================================\n");
            first = false;
        } else {
            printf(DSEPARATOR);
        }

        scanf(" ");

        point_t point;
        grid_clear(&grid);
        while(parse_point(&point)) {
            *grid_get(&grid, point) = BLOCK_CELL;
            grid.num_blocks++;
            scanf(" ");
        }
        status = verify_route(&grid, &path);
        iter_repair_route(&grid, &scratch_grid, &path, &status, true);
        scanf(" ");
    }

    printf(DSEPARATOR);

    #if DEBUG
    if((status == ROUTE_VALID || status == BLOCK_IN_ROUTE) && !feof(stdin)) {
        fprintf(stderr, "Warning: malformed input file.\n");
    }
    #endif

    ll_free(&path.steps);
    grid_free(&grid);
    grid_free(&scratch_grid);
	return 0;
}

void iter_repair_route(grid_t *grid, grid_t* scratch_grid, path_t* path, int *status, bool iter) {
    print_path_on_grid(grid, scratch_grid, path);

    /* if the grid wasn't broken, only print the grid with the valid route. */
    if(*status != BLOCK_IN_ROUTE) {
        return;
    }

    printf(SSEPARATOR);

    while(*status == BLOCK_IN_ROUTE) {
        grid_copy(grid, scratch_grid);
        bool repaired = repair_route(scratch_grid, path);

        if(!repaired) {
            break;
        }

        *status = verify_route(grid, path);
        if(*status == ROUTE_VALID || !iter) {
            /* if the route was broken and successfully repaired,
               or one repair was successful during stage one */
            print_path_on_grid(grid, scratch_grid, path);
            printf(SSEPARATOR);
            print_results(grid, path, *status, false);
            return;
        }
    }

    /* if the grid was broken and could not be repaired */
    print_path_on_grid(grid, scratch_grid, path);
    printf(SSEPARATOR);
    printf("The route cannot be repaired!\n");
}

int verify_route(grid_t *grid,
                 path_t *path) {
    assert(!ll_is_empty(&path->steps));
    if(!same_point(path->steps.head->coords, path->start)) {
        return INITIAL_CELL_WRONG;
    } else if(!same_point(path->steps.foot->coords, path->end)) {
        return GOAL_CELL_WRONG;
    }

    node_t *step = path->steps.head;
    bool block_in_route = false;
    while(step != NULL && step->next != NULL) {
        /* only allowed to move one cell in either direction each step */
        int difference = abs(step->coords.x - step->next->coords.x) +
                         abs(step->coords.y - step->next->coords.y);
        if(step->coords.x < 0 || step->coords.x >= grid->dim.cols
        || step->coords.y < 0 || step->coords.y >= grid->dim.rows) {
             return ILLEGAL_MOVE;
        }
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

/* repairs a route.
   `grid` should be a scratch grid which only contains block cells.
   if the function returns false, then the path will not be modified */
bool repair_route(grid_t *grid, path_t *path) {
    assert(!ll_is_empty(&path->steps));
    node_t *step = path->steps.head;
    /* get to the start of the repair */
    while(step->next != NULL && *grid_get(grid, step->next->coords) != BLOCK_CELL) {
        step = step->next;
    }

    node_t *repair_start = step;
    #if DEBUG
        fprintf(stderr, "Starting repair at: [%d, %d]", repair_start->coords.y, repair_start->coords.x);
    #endif

    /* skip the starting cell */
    if(step->next != NULL) {
        step = step->next;
    }
    /* skip the problem cells */
    while(step->next != NULL && *grid_get(grid, step->coords) == BLOCK_CELL) {
        step = step->next;
    }
    node_t *tail_path_start = step;

    /* our repair should find path segments *after* the broken region
       so we draw only those path segments onto the grid */
    draw_path(grid, tail_path_start);
    /* override the starting cell to be `0` */
    *grid_get(grid, repair_start->coords) = SEARCH_CELL_MIN;

    #if DEBUG
        fprintf(stderr, ", ending somewhere after: [%d, %d]\n", tail_path_start->coords.y, tail_path_start->coords.x);
    #endif

    vec_t repair;
    vec_init(&repair);
    vec_item_t search_start = {
        .coords = repair_start->coords,
        .distance = 0
    };
    vec_push(&repair, search_start);

    point_t repair_end;
    int dist;
    size_t i = 0;
    while(i < repair.len) {
        vec_item_t *current = vec_get(&repair, i);
        point_t p = current->coords;
        point_t above = { .x = p.x, .y = p.y - 1};
        point_t below = { .x = p.x, .y = p.y + 1};
        point_t left = { .x = p.x -1, .y = p.y };
        point_t right = { .x = p.x + 1, .y = p.y };
        dist = current->distance + 1;
        if((above.y >= 0 && try_repair_step(grid, above, dist, &repair, &repair_end)) ||
           (below.y < grid->dim.rows && try_repair_step(grid, below, dist, &repair, &repair_end)) ||
           (left.x >= 0 && try_repair_step(grid, left, dist, &repair, &repair_end)) ||
           (right.x < grid->dim.cols && try_repair_step(grid, right, dist, &repair, &repair_end))
          ) {
            break;
        }
        i++;
    }

    if(i >= repair.len) {
        return false;
    }
    vec_free(&repair);

    #if DEBUG
        fprintf(stderr, "Found route in %ld steps, ended at: [%d, %d]\n", i, repair_end.y, repair_end.x);
        print_grid(grid);
    #endif

    /* don't remove the starting cell */
    if(repair_start->next != NULL) {
        tail_path_start = repair_start->next;
    }
    /* remove parts of the old path, till we get to the end of the repair */
    while(tail_path_start->next != NULL && !same_point(tail_path_start->coords, repair_end)) {
        tail_path_start = tail_path_start->next;
        ll_remove(&path->steps, tail_path_start->prev);
    }

    point_t tail_point = repair_end;

    assert(*grid_get(grid, repair_start->coords) == SEARCH_CELL_MIN);

    /* if(same_point(tail_path_start->coords, repair_start->coords)) {
        ll_remove(&path->steps, tail_path_start);
    } */

    /* gradually add cells until the start of the tail path
       joins up with the start of the repair */
    while(!same_point(tail_point, repair_start->coords)) {
        #if DEBUG
            fprintf(stderr, "Backtracking from [%d, %d], towards: [%d, %d], current min distance %d\n", tail_point.y, tail_point.x, repair_start->coords.y, repair_start->coords.x, dist);
        #endif
        if(!same_point(tail_point, repair_end)) {
            tail_path_start = ll_insert_before(&path->steps, tail_path_start, tail_point);
        }

        point_t above = { .x = tail_point.x, .y = tail_point.y - 1};
        point_t below = { .x = tail_point.x, .y = tail_point.y + 1};
        point_t left = { .x = tail_point.x -1, .y = tail_point.y };
        point_t right = { .x = tail_point.x + 1, .y = tail_point.y };

        int old_dist = dist;
        if(right.x < grid->dim.cols) {
            try_backtrack(grid, &tail_point, &dist, right);
        }
        if(left.x >= 0) {
            try_backtrack(grid, &tail_point, &dist, left);
        }
        if(below.y < grid->dim.rows) {
            try_backtrack(grid, &tail_point, &dist, below);
        }
        if(above.y >= 0) {
            try_backtrack(grid, &tail_point, &dist, above);
        }

        /* sanity check that we're getting closer to start.
           otherwise this loop will not terminate. */
        if(dist >= old_dist) {
            fprintf(stderr, "Backtracking operation will not terminate.\n");
            exit(EXIT_FAILURE);
        }
    }


    return true;
}

bool try_repair_step(grid_t *grid, point_t point, int distance, vec_t *repair, point_t *repair_end) {
    int *cell = grid_get(grid, point);
    if(*cell == EMPTY_CELL) {
        vec_item_t next = { .distance = distance, .coords = point };
        *grid_get(grid, point) = SEARCH_CELL_MIN + next.distance;
        vec_push(repair, next);
    } else if(*cell == ROUTE_CELL) {
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

void draw_path(grid_t *grid, node_t *start_path) {
    /* draw the path into the 2D array*/
    node_t *step = start_path;
    /* if a visited cell is also the initial cell, goal cell, or contains a
       block, then `I`, `G`, or `#`, respectively, should be printed, not `*` */
    while(step != NULL) {
        cell_t *cell = grid_get(grid, step->coords);
        if(*cell != BLOCK_CELL) {
            *cell = ROUTE_CELL;
        }
        step = step->next;
    }
}

void print_path_on_grid(grid_t *grid, grid_t *scratch_grid, path_t *path) {
    grid_copy(grid, scratch_grid);
    draw_path(scratch_grid, path->steps.head);
    *grid_get(scratch_grid, path->start) = INITIAL_CELL;
    *grid_get(scratch_grid, path->end) = GOAL_CELL;
    print_grid(scratch_grid);
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
                else if(cell >= SEARCH_CELL_MIN) { printf("%d", (cell - SEARCH_CELL_MIN) % 10); }
            }
        }
        printf("\n");
    }
}

void print_point(point_t point) {
    printf("[%d,%d]", point.y, point.x);
}

void print_results(grid_t *grid, path_t *path, int status, bool initial) {
    if(initial) {
        printf("The grid has %d rows and %d columns.\n", grid->dim.rows, grid->dim.cols);
        printf("The grid has %d block(s).\n", grid->num_blocks);
        printf("The initial cell in the grid is ");
        print_point(path->start);
        printf(".\nThe goal cell in the grid is ");
        print_point(path->end);
        printf(".\nThe proposed route in the grid is:\n");
    }
    print_list(&path->steps);
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

void print_list(list_t *list) {
     node_t *step = list->head;
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
}

dimensions_t parse_dimensions() {
    dimensions_t dim;
    int res = scanf("%dx%d\n", &dim.rows, &dim.cols);
    if(res != 2) {
        fprintf(stderr, "Fatal error: failed to parse grid dimensions from stdin.\n");
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
void grid_init(grid_t *grid, dimensions_t dim) {
    grid->dim = dim;
    grid->num_blocks = 0;
    size_t bytes = sizeof(cell_t) * dim.rows * dim.cols;
    grid->inner = (cell_t *) malloc(bytes);
    assert(grid->inner != NULL);
    grid_clear(grid);
}

void grid_clear(grid_t *grid) {
    grid->num_blocks = 0;
    /* 'zeroes' the memory with empty cells */
    size_t bytes = sizeof(cell_t) * grid->dim.rows * grid->dim.cols;
    memset(grid->inner, EMPTY_CELL, bytes);
}

cell_t *grid_get(grid_t *grid, point_t point) {
    assert(grid->inner != NULL);
    assert(point.x >= 0 && point.x < grid->dim.cols
            && point.y >= 0 && point.y < grid->dim.rows);
    return &grid->inner[point.y * grid->dim.cols + point.x];
}

/* NOTE: if 'dest' was already an allocated grid, it must be freed before
   first otherwise memory will be leaked */
void grid_copy(grid_t *src, grid_t *dest) {
    assert(src->dim.cols == dest->dim.cols && src->dim.rows == dest->dim.rows);
    dest->num_blocks = src->num_blocks;
    size_t bytes = sizeof(cell_t) * src->dim.rows * src->dim.cols;
    memcpy(dest->inner, src->inner, bytes);
}

void grid_free(grid_t *grid) {
    free(grid->inner);
    grid->inner = NULL;
}

void ll_init(list_t* list) {
    list->head = NULL;
    list->foot = NULL;
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
}


void vec_init(vec_t *vec) {
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
        print_point(vec->storage[i].coords);
        printf(": #%d\n", vec->storage[i].distance);
    }
}

void vec_free(vec_t *vec) {
    free(vec->storage);
    vec->storage = NULL;
}